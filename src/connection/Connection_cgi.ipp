#pragma once
#include "Connection.hpp"

CONNECTION_INL
(pid_t) exec_script(char *const argv[3], int fdIn[2], int fdOut[2]) {
	bool fail = dup2(STDOUT_FILENO, fdOut[1]) == -1 || 
				dup2(STDIN_FILENO, fdIn[0]) == -1;

	close(fdOut[0]);	// Child Read End
	close(fdOut[1]);	// Parent Write End
	close(fdIn[0]);		// Child Read End
	close(fdIn[1]);		// Parent Write End
	if (fail) {
		close(STDOUT_FILENO);
		close(STDIN_FILENO);
		_exit(1);	// TODO: Appropriate return
	}

	execve(argv[0], argv, s_fakeEnv.envp);
	if (errno != ENOENT && errno != ENOTDIR)
		_exit(126);
	_exit(127);
}

/*
	Doing this before forking avoids Copy on Write. Fake env is good for that!
	REQUEST_METHOD=POST, SCRIPT_NAME=/cgi/test.py
	QUERY_STRING=a=1, CONTENT_LENGTH=42, CONTENT_TYPE=application/x-www-form-urlencoded
	HTTP_HOST=example.com:8080, HTTP_COOKIE=session=xyz
*/

CONNECTION_INL
(void) append_env(Buffer64 &buffer, char* argv[3]) {
	static const char requestMethod[3][24] = 
		{"REQUEST_METHOD=GET", "REQUEST_METHOD=POST", "REQUEST_METHOD=DELETE"};
	const usize methodIndex = (request.options & 7) / 2;

	s_fakeEnv.reset();
	Location &loc = cfg->locations[request.locationIndex];
	Span interpreterSpan = request.interpreter.extract(loc.cgiBlock.mptr());
	Span pathSpan = request.path.extract((char*) recvBuffer.data);
	Span querySpan = request.query.extract((char*) recvBuffer.data);
	Span cookieSpan = request.cookies.extract((char*) recvBuffer.data);

	argv[0] = buffer.append(loc.root.kptr(), loc.root.length);
	buffer.append(interpreterSpan);
	buffer.append("\0");
	char* scriptName = buffer.append("SCRIPT_NAME=");
	argv[1] = buffer.append(pathSpan);
	buffer.append("\0");
	argv[2] = NULL;

	querySpan.ptr = STRPREP(querySpan.ptr, "QUERY_STRING="); // Totally safe dont worry about it
	cookieSpan.ptr = STRPREP(cookieSpan.ptr, "HTTP_COOKIE="); // Worst case scenario overwrites HTTP/1.1\r\n

	if (request.options & Options::FIXED_LENGTH) {
		char* lengthStr = buffer.append("HTTP_CONTENT_LENGTH=");
		buffer.append_digit10(request.bodySize);
		buffer.append("\0");	// TODO: Check if append null terminates
		s_fakeEnv.append(lengthStr);
	}
	s_fakeEnv.append(requestMethod[methodIndex]);
	s_fakeEnv.append(scriptName);
	s_fakeEnv.append(querySpan.ptr);
	s_fakeEnv.append(cookieSpan.ptr);
}

CONNECTION_INL
(isize) cgi_first_run() {
	char *argv[3];
	Buffer64 buffer;
	int fdIn[2], fdOut[2];

	if (pipe(fdIn) == -1)
		goto Error;
	if (pipe(fdOut) == -1)
		goto ErrorCloseInput;
	if (VirtualServer::s_set_nonblocking(fdOut[0]) || VirtualServer::s_set_nonblocking(fdIn[1]))
		goto ErrorCloseOutput;

	append_env(buffer, argv);
	processId = fork();
	if (processId < 0)
		goto ErrorCloseOutput;
	if (processId == 0)
		exec_script(argv, fdIn, fdOut);

	close(fdIn[0]);
	close(fdOut[1]);
	readFd = fdOut[0];
	writeFd = fdIn[1];
	return 0;

	ErrorCloseOutput:
		close(fdOut[0]);
		close(fdOut[1]);
	ErrorCloseInput:
		close(fdIn[0]);
		close(fdIn[1]);
	Error:
		return -1;
}

/*
	The pipe fds here are configured to be non-blocking and read/write errors are ignored
	Failure conditions for these fds are instead handled by CGI timeouts
*/

CONNECTION_INL
(isize) cgi_method() {
	isize bytesWritten, bytesRead;

	if (request.options & Options::CHUNKED_LENGTH)
		bytesWritten = recvBuffer.decode(writeFd, request.chunkSize, request.bodySize);
	else {
		bytesWritten = recvBuffer.write(writeFd, request.bodySize);
		if (bytesWritten > 0)
			request.bodySize -= (usize) bytesWritten;
	}

	if (bytesWritten == -1) {
		close(writeFd);
		writeFd = -1;
		request.status = Status::i500;
		return error_path();
	}

	if (request.bodySize == 0) {	// Must guarantee that bodySize is 0
		close(writeFd);
		writeFd = -1;	// Finished reading
	}

	bytesRead = sendBuffer.read(readFd, ATOMIC_IOSIZE);
	if (bytesRead == 0) {
		close(readFd);
		readFd = -1;
	}

	isize delta = ((bytesWritten < 0 || bytesRead < 0) ? -1 : 1);
	bonusTime = CLAMP(bonusTime + delta, 0, 30);

	if (!request.status.is_set()) {
		if (sendBuffer.find_header_end() != SIZE_MAX) {
			if (sendBuffer.is_full())
				return -1;
			return 0;	// Still no CGI Header
		}
		build_cgi_header();
	}
	return bytesRead;
}

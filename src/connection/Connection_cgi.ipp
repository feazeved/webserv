#pragma once
#include "Connection.hpp"

CONNECTION_INL
(void) exec_script(char* cgiPath, char* scriptPath, int fdIn[2], int fdOut[2]) {
	char *const argv[3] = {cgiPath, scriptPath, NULL};

	bool fail = dup2(STDOUT_FILENO, fdOut[1]) == -1 || 
				dup2(STDIN_FILENO, fdIn[0]) == -1;

	close(fdOut[0]);	// Child Read End
	close(fdOut[1]);	// Parent Write End
	close(fdIn[0]);	// Child Read End
	close(fdIn[1]);	// Parent Write End
	if (fail) {
		close(STDOUT_FILENO);
		close(STDIN_FILENO);
		_exit(1);	// TODO: Appropriate return
	}

	execve(cgiPath, (char* const*) argv, g_fakeEnv.envp);
	if (errno != ENOENT && errno != ENOTDIR)
		_exit(126);
	_exit(127);
}

/*
	Doing this before forking avoids Copy on Write. Fake env is good for that!
	REQUEST_METHOD=POST
	SCRIPT_NAME=/cgi/test.py
	QUERY_STRING=a=1
	CONTENT_LENGTH=42
	CONTENT_TYPE=application/x-www-form-urlencoded

	HTTP_HOST=example.com:8080
	HTTP_COOKIE=session=xyz
*/

CONNECTION_INL
(void) append_env(char* scriptName) {
	static char contentLength[48] = "HTTP_CONTENT_LENGTH=";
	static const char requestMethod[3][24] = 
		{"REQUEST_METHOD=GET", "REQUEST_METHOD=POST", "REQUEST_METHOD=DELETE"};
	const usize methodIndex = (request.options & 7) / 2;

	Span pathSpan = request.path.extract((char*) recvBuffer.data);
	Span querySpan = request.query.extract((char*) recvBuffer.data);
	Span cookieSpan = request.cookies.extract((char*) recvBuffer.data);
	
	g_fakeEnv.reset();
	MEMCPY(scriptName + 12, pathSpan.ptr, pathSpan.length);
	scriptName[12 + pathSpan.length] = 0;

	querySpan.ptr -= 13;	// Inplace changes query, overrides path
	MEMCPY_INLINE(querySpan.ptr, "QUERY_STRING=", 13); // Totally safe dont worry about it

	cookieSpan.ptr -= 12;
	MEMCPY_INLINE(cookieSpan.ptr, "HTTP_COOKIE=", 12); // Worst case scenario overrides HTTP/1.1\r\n

	if (request.options & Options::FIXED_LENGTH) {
		char buffer[48];
		// itoa
		// MEMCMP_INLINE(contentLength + 20, );
		s_fakeEnv.append(contentLength);
	}
	s_fakeEnv.append(requestMethod[methodIndex]);
	s_fakeEnv.append(scriptName);
	s_fakeEnv.append(querySpan.ptr);
	s_fakeEnv.append(cookieSpan.ptr);
}

// TODO: parsing needs to build the path for the script
CONNECTION_INL
(isize) cgi_first_run() {
	char pathBuffer[8192];
	char scriptName[8192] = "SCRIPT_NAME=";
	char* ptr = request.path.index + (char*) recvBuffer.data;
	Location &loc = cfg->locations[request.locationIndex];
	Span interpSpan = request.interpreter.extract(loc.cgiBlock.mptr());

	int fdIn[2];
	int fdOut[2];

	if (pipe(fdIn) == -1)
		goto Error;
	if (pipe(fdOut) == -1)
		goto ErrorCloseInput;
	if (VirtualServer::s_set_nonblocking(fdOut[0]) || VirtualServer::s_set_nonblocking(fdIn[1]))
		goto ErrorCloseOutput;

	s_build_path(pathBuffer, ptr, request.path.length, loc.root);
	append_env(scriptName);
	processId = fork();
	if (processId < 0)
		goto ErrorCloseOutput;
	if (processId == 0)
		exec_script(pathBuffer, interpSpan.ptr, fdIn, fdOut);

	close(fdIn[0]);
	close(fdOut[1]);
	readFd = fdOut[0];
	writeFd = fdIn[1];
	// cgiStartTime = g_timeNow;
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

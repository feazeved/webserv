#pragma once
#include "Connection.hpp"

/*	The pipe fds here are configured to be non-blocking and read/write errors are ignored
	Failure conditions for these fds are instead handled by CGI timeouts
*/

CONNECTION_INL
(isize) cgi(Epoll &epoll) {
	isize bytesWritten, bytesRead;

	if (options & Options::CHUNKED_LENGTH)
		bytesWritten = recvBuffer.decode(writeFd, chunkSize, bodySize);
	else {
		bytesWritten = recvBuffer.write(writeFd, bodySize);
		if (bytesWritten > 0)
			bodySize -= (usize) bytesWritten;
	}

	if (bytesWritten == -1) {
		close(writeFd);
		writeFd = -1;
		return flush_setup_close(epoll, Status::i500);
	}

	bytesRead = sendBuffer.read(readFd, ATOMIC_IOSIZE);
	if (bytesRead == 0) {
		close(readFd);
		readFd = -1;
	}

	// isize delta = ((bytesWritten < 0 || bytesRead < 0) ? -1 : 1);

	if (bodySize == 0) {
		Span header = sendBuffer.find_header_end();
		if (header.ptr == NULL) {
			if (sendBuffer.is_full())
				return -1;
			return 0;	// Still no CGI Header
		}
		Status::Code code = build_cgi_header(Status::i200);
		close(writeFd);
		writeFd = -1;	// Finished reading
		if (code >= Status::i400)
			flush_setup_close(epoll, code);
		flush_setup(epoll, code);
	}
	return bytesRead;
}

static inline
void s_exec_script(char *const argv[3], char **envp, int fdIn[2], int fdOut[2], char* cwdPath) {
	bool fail = dup2(fdOut[1], STDOUT_FILENO) == -1;
	fail = fail || dup2(fdIn[0], STDIN_FILENO) == -1;

	close(fdOut[0]), close(fdOut[1]);
	close(fdIn[0]), close(fdIn[1]);
	if (fail || chdir(cwdPath) == -1) {
		close(STDOUT_FILENO), close(STDIN_FILENO);
		_exit(1);
	}

	execve(argv[0], argv, envp);
	if (errno != ENOENT && errno != ENOTDIR)
		_exit(126);
	_exit(127);
}

static inline
void s_chdir(char* cwdPath, usize length) {
	char* slashPtr = NULL;
	char* end = cwdPath + length;

	*end = '/';
	while (true) {
		while (*cwdPath != '/')
			cwdPath++;
		if (cwdPath >= end)
			break;
		slashPtr = cwdPath++;
	}
	*slashPtr = 0;
	ASSERT(cwdPath != NULL, "cwdPath was NULL");
}

/*
	Doing this before forking avoids Copy on Write. Fake env is good for that!
	REQUEST_METHOD=POST, SCRIPT_NAME=/cgi/test.py
	QUERY_STRING=a=1, CONTENT_LENGTH=42, CONTENT_TYPE=application/x-www-form-urlencoded
	HTTP_HOST=example.com:8080, HTTP_COOKIE=session=xyz
*/

CONNECTION_INL
(char*) append_env(Buffer64 &buffer, char* argv[3]) {
	static const char requestMethod[3][24] = 
		{"REQUEST_METHOD=GET", "REQUEST_METHOD=POST", "REQUEST_METHOD=DELETE"};
	const usize methodIndex = (options & 7) / 2;

	Environment::reset();

	argv[0] = buffer.append(req.interpreter.ptr, req.interpreter.size + 1);			// /bin/python3
	char* scriptName = buffer.append("SCRIPT_NAME=");	// SCRIPTNAME=
	buffer.append(req.target.ptr, req.target.size + 1);							// SCRIPTNAME=/images/cgi/process.py
	argv[1] = append_target_path(buffer);
	argv[2] = NULL;
	
	const usize scriptPathLength = (usize)(buffer.wptr() - argv[1]);
	buffer.writePos++;
	char* cwdPath = buffer.append(argv[1], scriptPathLength + 1);
	s_chdir(cwdPath, scriptPathLength);							// /home/webserv/www/images/cgi

	Environment::append(STRPREP(req.query.ptr, "QUERY_STRING="));
	if (req.cookies.size != 0)
		Environment::append(STRPREP(req.cookies.ptr, "HTTP_COOKIE="));
	Environment::append(buffer.append("HTTP_HOST="));
	buffer.append(req.host.ptr, req.host.size + 1);

	if (options & Options::FIXED_LENGTH) {
		char* lengthStr = buffer.append("CONTENT_LENGTH=");
		buffer.append_digit10(bodySize);
		buffer.append("\0");
		Environment::append(lengthStr);
	}
	if (req.contentTypeHeader.size != 0) {
		char* contentTypeHeader = buffer.append("CONTENT_TYPE=");
		buffer.append(req.contentTypeHeader);
		buffer.append("\0");
		Environment::append(contentTypeHeader);
	}
	Environment::append((char*) requestMethod[methodIndex]);
	Environment::append(scriptName);
	return cwdPath;
}

CONNECTION_INL
(isize) cgi_setup(Epoll &epoll) {
	Buffer64 pathBuffer = {};
	char *chdirPath;
	char *argv[3];
	int fdIn[2], fdOut[2];

	chdirPath = append_env(pathBuffer, argv);
	struct stat scriptStat;
	if (stat(argv[1], &scriptStat) == -1 || access(argv[1], R_OK) == -1)
		goto Error;
	if (!S_ISREG(scriptStat.st_mode))
		return flush_setup_close(epoll, Status::i403);

	if (pipe(fdIn) == -1)
		goto Error;
	if (pipe(fdOut) == -1)
		goto ErrorCloseInput;
	if (fn::set_stream_mode(fdIn[1]) || fn::set_stream_mode(fdOut[0]))
		goto ErrorCloseOutput;
	processId = fork();
	if (processId < 0)
		goto ErrorCloseOutput;
	if (processId == 0)
		s_exec_script(argv, Environment::envp, fdIn, fdOut, chdirPath);

	close(fdIn[0]);
	close(fdOut[1]);
	readFd = fdOut[0];
	writeFd = fdIn[1];
	sendBuffer.clear();
	return cgi(epoll);

	ErrorCloseOutput:	close(fdOut[0]), close(fdOut[1]);
	ErrorCloseInput:	close(fdIn[0]), close(fdIn[1]);
	Error:				return flush_setup_close(epoll, s_get_status());
}

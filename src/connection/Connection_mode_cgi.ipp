#pragma once
#include "Connection.hpp"

/*	The pipe fds here are configured to be non-blocking and read/write errors are ignored
	Failure conditions for these fds are instead handled by CGI timeouts
*/

CONNECTION_INL
(isize) switch_to_cgi(Epoll &epoll) {
	close(writeFd);
	writeFd = -1;
	bodySize = recvBuffer.scanPos - recvBuffer.readPos;
	mode = Mode::CGI;
	if (epoll.modify(clientFd, EPOLLOUT, epollState))
		return -1;
	return read_from_client(epoll);
}

CONNECTION_INL
(isize) cgi_chunked(Epoll &epoll) {
	if (recvBuffer.readPos < recvBuffer.scanPos) {
		usize bytesLeft = recvBuffer.scanPos - recvBuffer.readPos;
		if (recvBuffer.atomic_write(writeFd, bytesLeft, bodySize) == -1)
			return 0;
		if (recvBuffer.readPos < recvBuffer.scanPos)
			return 0;
	}

	HTTP_Buffer tmpBuffer = {};
	Status::Code code = recvBuffer.dechunk(tmpBuffer, chunkSize, bodySize);
	if (tmpBuffer.size() != 0) {
		tmpBuffer.atomic_write(writeFd, bodySize, bodySize);
		const usize decodedRemaining = tmpBuffer.size();
		const usize rawRemaining = recvBuffer.size();
		if (rawRemaining != 0)
			tmpBuffer.append(recvBuffer.rptr(), rawRemaining);
		recvBuffer.bufcpy(tmpBuffer);
		recvBuffer.scanPos = decodedRemaining;
	}

	if (code == Status::ok)
		switch_to_cgi(epoll);
	return read_from_client(epoll);
}

CONNECTION_INL
(isize) cgi_fixed(Epoll &epoll) {
	recvBuffer.atomic_write(writeFd, bodySize, bodySize);
	if (recvBuffer.size() < bodySize)
		return -1;
	if (recvBuffer.size() >= bodySize)
		switch_to_cgi(epoll);
	return read_from_client(epoll);
}

CONNECTION_INL
(isize) cgi(Epoll &epoll) {
	isize bytesRead = 0;
	if (readFd >= 0) {
		bytesRead = sendBuffer.read(readFd, ATOMIC_IOSIZE);
		if (bytesRead == 0) {
			close(readFd);
			readFd = -1;
		}
	}

	if (!status.is_set()) {
		Span header = sendBuffer.find_header_end();
		if (header.ptr == NULL) {
			if (bytesRead == -2 || readFd == -1)
				return flush_setup_close(epoll, Status::i500);
			return 0;	// Still no CGI Header
		}
		Status::Code code = build_cgi_header(Status::i200);
		if (code == Status::ixxx)
			return flush_setup_close(epoll, Status::i500);
		status = code;
	}
	if (readFd == -1)
		return flush_setup(epoll, (Status::Code)status.index);
	return write_to_client(epoll);
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
char* s_split_filename(char* cwdPath, usize length) {
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
	ASSERT(cwdPath != NULL, "cwdPath was NULL");
	*slashPtr = 0;
	*end = 0;
	return slashPtr + 1;
}

/*
	Doing this before forking avoids Copy on Write. Fake env is good for that!
	REQUEST_METHOD=POST, SCRIPT_NAME=/cgi/test.py
	QUERY_STRING=a=1, CONTENT_LENGTH=42, CONTENT_TYPE=application/x-www-form-urlencoded
	HTTP_HOST=example.com:8080, HTTP_COOKIE=session=xyz
*/
// TODO: Review and write what it is supposed to do
CONNECTION_INL
(char*) append_env(Buffer64 &buffer, char* argv[3]) {
	static const char requestMethod[3][24] = 
		{"REQUEST_METHOD=GET", "REQUEST_METHOD=POST", "REQUEST_METHOD=DELETE"};
	const usize methodIndex = (options & 7) / 2;

	Environment::reset();

	char* scriptName = buffer.append("SCRIPT_NAME=");	// SCRIPTNAME=
	buffer.append(req.target.ptr, req.target.size + 1);	// SCRIPTNAME=/images/cgi/process.py
	char* scriptPath = append_target_path(buffer);		// /home/webserv/www/images/cgi/process.py
	const usize scriptPathLength = (usize)(buffer.wptr() - scriptPath);
	buffer.writePos++;
	struct stat st;
	if (stat(scriptPath, &st) == -1 || access(scriptPath, R_OK) == -1)
		return NULL;
	// if (!S_ISREG(st.st_mode))
	// 	return flush_setup_close(epoll, Status::i403);
	char* cwdPath = buffer.append(scriptPath, scriptPathLength + 1);			// /home/webserv/www/images/cgi
	argv[0] = buffer.append(req.interpreter.ptr, req.interpreter.size + 1);			// /bin/python3
	argv[1] = s_split_filename(cwdPath, scriptPathLength);						// process.py
	argv[2] = NULL;
	Environment::append(buffer.append("HTTP_HOST="));
	buffer.append(req.host.ptr, req.host.size + 1);
	Environment::append(STRPREP(req.query.ptr, "QUERY_STRING="));
	if (req.cookies.size != 0)
		Environment::append(STRPREP(req.cookies.ptr, "HTTP_COOKIE="));

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
	if (chdirPath == NULL)
		goto Error;
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
	if (mode == Mode::CGI_FIXED)
		return cgi_fixed(epoll);
	if (mode == Mode::CGI_CHUNKED)
		return cgi_chunked(epoll);
	ASSERT(mode == Mode::CGI, "Invalid CGI mode");
	return cgi(epoll);

	ErrorCloseOutput:	close(fdOut[0]), close(fdOut[1]);
	ErrorCloseInput:	close(fdIn[0]), close(fdIn[1]);
	Error:				return flush_setup_close(epoll, s_get_status());
}

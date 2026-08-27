#pragma once
#include "Connection.hpp"

CONNECTION_INL
(isize) del_setup() {
	Buffer16 pathBuffer;
	pathBuffer.append(req.location->root.extract());
	pathBuffer.append(req.path);
	pathBuffer.append("\0");

	struct stat st;
	if (stat(pathBuffer, &st) == -1)
		return s_get_status(status);
	if (S_ISDIR(st.st_mode))
		return s_get_status(status);	// Forbids deleting directories
	if (unlink(pathBuffer) == -1)
		return s_get_status(status);

	status = Status::i204;
	build_header();
	return 0;
}

CONNECTION_INL
(isize) post_setup() {
	Buffer16 pathBuffer;
	pathBuffer.append(req.location->root.extract());
	pathBuffer.append(req.location->uploadStore.extract());
	pathBuffer.append("\0");

	writeFd = open(pathBuffer, O_WRONLY | O_CREAT | O_EXCL, 0644);
	if (writeFd == -1) {
		mode = Mode::CLOSE;
		return s_get_status(status);
	}
	return 0;
}

#define HTTP_INDEX_HEADER "<html><head><title>Index of /download/</title></head><body><h1>Index of "
#define HTTP_INDEX_MIDDLE "/</title></head><body><h1>Index of "
#define HTTP_INDEX_TAIL "/</h1><hr><pre><a href=\"../\">../</a>"

CONNECTION_INL
(isize) get_directory(struct stat &st, Buffer8 &pathBuffer) {
	pathBuffer.append("/index.html");
	readFd = open(pathBuffer, O_RDONLY);
	if (readFd == -1 && req.location->autoindex == false)
		return s_get_status(status);
	if (readFd == -1) {
		pathBuffer.writePos -= sizeof("/index.html");	// TODO: add overwrite function
		*pathBuffer = 0;
		directory = opendir(pathBuffer);
		if (directory == NULL)
			return s_get_status(status);
	}
	status = Status::i200;
	bodySize = (usize)st.st_size;
	build_header();
	sendBuffer.append(HTTP_INDEX_HEADER);
	sendBuffer.append(req.path);			// TODO: Actually might need to be the last /
	sendBuffer.append(HTTP_INDEX_MIDDLE);
	sendBuffer.append(req.path);
	sendBuffer.append(HTTP_INDEX_TAIL);
	return 0;
}

CONNECTION_INL
(isize) get_setup() {
	Buffer8 pathBuffer;
	pathBuffer.append(req.location->root.extract());
	pathBuffer.append(req.path);
	pathBuffer.append("\0");

	struct stat st;
	if (stat(pathBuffer, &st) == -1)
		return s_get_status(status);

	if (S_ISDIR(st.st_mode))
		return get_directory(st, pathBuffer);
	readFd = open(pathBuffer, O_RDONLY);
	if (readFd == -1)
		return s_get_status(status);
	status = Status::i200;
	bodySize = (usize)st.st_size;
	build_header();
	return 0;
}

CONNECTION_INL
(isize) cgi_setup() {
	Buffer64 pathBuffer;

	char *argv[3];
	int fdIn[2], fdOut[2];

	if (pipe(fdIn) == -1)
		goto Error;
	if (pipe(fdOut) == -1)
		goto ErrorCloseInput;
	if (VirtualServer::s_set_nonblocking(fdOut[0]) || VirtualServer::s_set_nonblocking(fdIn[1]))
		goto ErrorCloseOutput;

	append_env(pathBuffer, argv);
	processId = fork();
	if (processId < 0)
		goto ErrorCloseOutput;
	if (processId == 0)
		s_exec_script(argv, cfg->s_fakeEnv.envp, fdIn, fdOut);

	close(fdIn[0]);
	close(fdOut[1]);
	readFd = fdOut[0];
	writeFd = fdIn[1];
	return 0;

	ErrorCloseOutput:	close(fdOut[0]); close(fdOut[1]);
	ErrorCloseInput:	close(fdIn[0]); close(fdIn[1]);
	Error:				return -1;
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
	const usize methodIndex = (options & 7) / 2;

	cfg->s_fakeEnv.reset();

	argv[0] = buffer.append(req.location->root.extract());
	buffer.append(req.interpreter);
	buffer.append("\0");	// Todo: check if these params are null terminated
	char* scriptName = buffer.append("SCRIPT_NAME=");
	argv[1] = buffer.append(req.path);
	buffer.append("\0");
	argv[2] = NULL;

	req.query.ptr = STRPREP(req.query.ptr, "QUERY_STRING="); // Totally safe dont worry about it
	req.cookies.ptr = STRPREP(req.cookies.ptr, "HTTP_COOKIE="); // Worst case scenario overwrites HTTP/1.1\r\n

	if (options & Options::FIXED_LENGTH) {
		char* lengthStr = buffer.append("HTTP_CONTENT_LENGTH=");
		buffer.append_digit10(bodySize);
		buffer.append("\0");	// TODO: Check if append null terminates
		cfg->s_fakeEnv.append(lengthStr);
	}
	cfg->s_fakeEnv.append(requestMethod[methodIndex]);
	cfg->s_fakeEnv.append(scriptName);
	cfg->s_fakeEnv.append(req.query.ptr);
	cfg->s_fakeEnv.append(req.cookies.ptr);
}

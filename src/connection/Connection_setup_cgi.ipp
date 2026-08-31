#pragma once
#include "Connection.hpp"

static inline
pid_t s_exec_script(char *const argv[3], char **envp, int fdIn[2], int fdOut[2], char* cwdPath) {
	bool fail = dup2(fdOut[1], STDOUT_FILENO) == -1;
	fail = fail || dup2(fdIn[0], STDIN_FILENO) == -1;

	close(fdOut[0]), close(fdOut[1]);
	close(fdIn[0]), close(fdIn[1]);
	if (fail || chdir(cwdPath) == -1) {
		close(STDOUT_FILENO), close(STDIN_FILENO);
		std::exit(1);
	}

	execve(argv[0], argv, envp);
	if (errno != ENOENT && errno != ENOTDIR)
		std::exit(126);
	std::exit(127);
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
		slashPtr = cwdPath;
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

/*
	Suppose we have /images/cgi/process.py, the URI is /images, and our root is /home/webserv/www/
	First, we match the upper path with the URI and find a matching location URI

	Interpreter path: /bin/python3, goes in argv[0]
	chdir to: /home/webserv/www/images/cgi
*/

CONNECTION_INL
(char*) append_env(Buffer64 &buffer, char* argv[3]) {
	static const char requestMethod[3][24] = 
		{"REQUEST_METHOD=GET", "REQUEST_METHOD=POST", "REQUEST_METHOD=DELETE"};
	const usize methodIndex = (options & 7) / 2;

	Environment::reset();
	Span root = req.location->get_root();

	argv[0] = buffer.append(req.interpreter);			// /bin/python3
	char* scriptName = buffer.append("SCRIPT_NAME=");	// SCRIPTNAME=
	buffer.append(req.target);							// SCRIPTNAME=/images/cgi/process.py
	argv[1] = buffer.append(root);						// /home/webserv/www
	buffer.append(req.target);							// /home/webserv/www/images/cgi/process.py
	argv[2] = NULL;

	usize scriptPathLength = req.target.size + root.size;
	char* cwdPath = buffer.append(argv[1], scriptPathLength);
	s_chdir(cwdPath, scriptPathLength);							// /home/webserv/www/images/cgi

	req.query.ptr = STRPREP(req.query.ptr, "QUERY_STRING="); // Totally safe dont worry about it
	req.cookies.ptr = STRPREP(req.cookies.ptr, "HTTP_COOKIE="); // Worst case scenario overwrites HTTP/1.1\r\n

	if (options & Options::FIXED_LENGTH) {
		char* lengthStr = buffer.append("HTTP_CONTENT_LENGTH=");
		buffer.append_digit10(bodySize);
		buffer.append("\0");				// TODO: Check if append null terminates
		Environment::append(lengthStr);
	}
	Environment::append((char*) requestMethod[methodIndex]);
	Environment::append(scriptName);
	Environment::append(req.query.ptr);
	Environment::append(req.cookies.ptr);
	return cwdPath;
}

CONNECTION_INL
(isize) cgi_setup() {
	Buffer64 pathBuffer;
	char *chdirPath;
	char *argv[3];
	int fdIn[2], fdOut[2];

	if (pipe(fdIn) == -1)
		goto Error;
	if (pipe(fdOut) == -1)
		goto ErrorCloseInput;
	if (VirtualServer::s_set_nonblocking(fdOut[0]) || VirtualServer::s_set_nonblocking(fdIn[1]))
		goto ErrorCloseOutput;

	chdirPath = append_env(pathBuffer, argv);
	processId = fork();
	if (processId < 0)
		goto ErrorCloseOutput;
	if (processId == 0)
		s_exec_script(argv, Environment::envp, fdIn, fdOut, chdirPath);

	close(fdIn[0]);
	close(fdOut[1]);
	readFd = fdOut[0];
	writeFd = fdIn[1];
	return 0;

	ErrorCloseOutput:	close(fdOut[0]), close(fdOut[1]);
	ErrorCloseInput:	close(fdIn[0]), close(fdIn[1]);
	Error:				return s_get_status(status);
}

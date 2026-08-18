#pragma once

#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include "core.hpp"
#include "core_macros.ipp"
#include "http/Buffer.hpp"
#include "http/Connection.hpp"
#include "Environment.hpp"

extern time_t g_timeNow;
extern HTTP::Environment g_fakeEnv;

namespace HTTP {
static inline
bool s_set_noblock(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		return false;

	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
		return false;

	return true;
}

static inline
void s_exec_script(u8 cgiType, const char *scriptPath, int fdIn[2], int fdOut[2]) {
	static const char *cgiPath[] = {"/usr/bin/python3", "/usr/bin/other"};
	const char *const argv[3] = {cgiPath[cgiType], scriptPath, NULL};

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

	// TODO: assumes path was built on clientOutput
	// TODO: assumes cgiType is determined on parsing
	execve(cgiPath[cgiType], (char* const*) argv, g_fakeEnv.envp);
	if (errno != ENOENT && errno != ENOTDIR)
		_exit(126);
	_exit(127);
}

/*
	Doing this before forking avoids Copy on Write. Fake env is good for that!
	REQUEST_METHOD=POST
	QUERY_STRING=a=1
	CONTENT_LENGTH=42
	HTTP_COOKIE=session=xyz
*/

// GET a.py?

static inline
void s_append_env(u8* buffer, Request &request) {
	static u8 scriptName[256 + 64] = "SCRIPT_NAME=";	// Making it static avoids having to copy SCRIPT_NAME=
	static u8 requestMethod[32] = "REQUEST_METHOD=";
	static const u8 methods[3][8] = {"GET", "POST", "DELETE"};

	g_fakeEnv.reset();
	MEMCPY(scriptName + 12, buffer + request.path.index, request.path.size + 1);
	MEMCPY_INLINE(requestMethod + 15, methods[(request.mode & 7) - 1], 8);			// TODO: triple check the enums

	u8* query = (buffer + request.query.index) - 13;	// Inplace changes query, overrides path
	MEMCPY_INLINE(query, "QUERY_STRING=", 13);			// Totally safe dont worry about it, buffer is padded

	u8* cookies = (buffer + request.cookies.index) - 12;	// Inplace changes Cookies
	MEMCPY_INLINE(cookies, "HTTP_COOKIE=", 12);			// Worst case scenario overrides HTTP/1.1\r\n

	g_fakeEnv.append(scriptName);
	g_fakeEnv.append(query);
	g_fakeEnv.append(cookies);
	g_fakeEnv.append((u8*) NULL);
}

// TODO: parsing needs to build the path for the script
CONNECTION_INL
(isize) cgi_first_run() {
	int fdIn[2];
	int fdOut[2];

	if (pipe(fdIn) == -1)
		goto Error;
	if (pipe(fdOut) == -1)
		goto ErrorCloseInput;
	if (s_set_noblock(fdOut[0]) == false || s_set_noblock(fdIn[1]) == false)
		goto ErrorCloseOutput;

	s_append_env(recvBuffer.cursor.memStart, request);
	processId = fork();
	if (processId < 0)
		goto ErrorCloseOutput;
	if (processId == 0)
		s_exec_script(request.cgiType, (char*)recvBuffer.cursor.memStart, fdIn, fdOut);

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
	isize bytesRead, bytesWritten;

	bytesWritten = write_to_server();
	bytesRead = read_from_server();

	isize delta = ((bytesWritten < 0 || bytesRead < 0) ? -1 : 1);
	bonusTime = CLAMP(bonusTime + delta, 0, 30);

	// Return path until the operation isnt complete
	if (request.status.is_set()) {
		if (sendBuffer.cursor.find_header_end() == false) {
			if (sendBuffer.cursor.is_full())
				return -1;	// ERROR: CGI Header is too big
			return 0;	// Still no CGI Header
		}
		build_cgi_header();
	}
	return bytesRead;
}
}

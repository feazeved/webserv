#pragma once

#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include "core.hpp"
#include "core_macros.ipp"
#include "http/Buffer.hpp"
#include "http/Connection.hpp"

extern time_t g_timeNow;

namespace HTTP {
static inline
isize s_close_all(int *fdInput, int *fdOutput) {
	close(fdInput[0]);	// Child Read End
	close(fdInput[1]);	// Parent Write End
	if (fdOutput != NULL) {
		close(fdOutput[0]);	// Parent Read End
		close(fdOutput[1]);	// Child Write End		
	}
	return -1;
}

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

CONNECTION_INL
(isize) cgi_first_run() {
	int fdInput[2];
	int fdOutput[2];

	if (pipe(fdInput) == -1)
		return -1;
	if (pipe(fdOutput) == -1)
		return s_close_all(fdInput, NULL);
	if (s_set_noblock(fdOutput[0]) == false || s_set_noblock(fdInput[1]) == false)
		return s_close_all(fdInput, fdOutput);

	processId = fork();
	if (processId < 0)
		return s_close_all(fdInput, fdOutput);

	if (processId == 0) {
		bool fail = dup2(STDOUT_FILENO, fdOutput[1]) == -1 || dup2(STDIN_FILENO, fdInput[0]) == -1;
		s_close_all(fdInput, fdOutput);
		if (fail) {
			close(STDOUT_FILENO);
			close(STDIN_FILENO);
			_exit(1);	// TODO: Appropriate return
		}
		// execve
		_exit(1);
	}

	close(fdInput[0]);
	close(fdOutput[1]);
	readFd = fdOutput[0];
	writeFd = fdInput[1];
	cgiStartTime = g_timeNow;
}

/*	The pipe fds here are configured to be non-blocking and read/write errors are ignored
	Failure conditions for these fds are instead handled by CGI timeouts */
CONNECTION_INL
(isize) cgi_method() {
	isize bytesRead, bytesWritten;

	bytesWritten = write_to_server();
	bytesRead = read_from_server();

	isize delta = ((bytesWritten < 0 || bytesRead < 0) ? -1 : 1);
	bonusTime = CLAMP(bonusTime + delta, 0, 30);

	// Return path until the operation isnt complete
	if (request.status.is_set()) {
		if (clientOutput.cursor.find_header_end() == false) {
			if (clientOutput.cursor.is_full())
				return -1;	// ERROR: CGI Header is too big
			return 0;	// Still no CGI Header
		}
		build_cgi_header();
	}
	return bytesRead;
}
}

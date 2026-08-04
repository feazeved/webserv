#pragma once

#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include "core.hpp"
#include "http/Request.hpp"

namespace HTTP {

template <usize bufferSize> inline
isize Request<bufferSize>::cgi_first_run() {
	int fdInput[2];
	int fdOutput[2];

	if (pipe(fdInput) == -1)
		return -1;
	if (pipe(fdOutput) == -1) {
		close(fdInput[0]);	// Child Read End
		close(fdInput[1]);	// Parent Write End
		return -1;
	}

	processId = fork();
	if (processId < 0) {
		close(fdInput[0]);	// Child Read End
		close(fdInput[1]);	// Parent Write End
		close(fdOutput[0]);	// Parent Read End
		close(fdOutput[1]);	// Child Write End
		return -1;
	}

	if (processId == 0) {
		bool fail = dup2(STDOUT_FILENO, fdOutput[1]) == -1 || dup2(STDIN_FILENO, fdInput[0]) == -1;
		close(fdInput[1]);
		close(fdOutput[0]);
		close(fdInput[0]);
		close(fdOutput[1]);
		if (fail) {
			close(STDOUT_FILENO);
			close(STDIN_FILENO);
			return -1;
		}
		// execve
		_exit(1);
	}

	close(fdInput[0]);
	close(fdOutput[1]);
	fd.readEnd = fdOutput[0];
	fd.writeEnd = fdInput[1];
	// Write once to CGI
	// Start timer
}

/*
The pipe fds here are configured to be non-blocking and read/write errors are ignored
Failure conditions for these fds are instead handled by CGI timeouts
*/
template <usize bufferSize> inline
isize Request<bufferSize>::cgi_method(usize bytes, u32 events) {
	isize bytesRead, bytesWritten;

	bytesRead = read_from_client(bytes, events);
	if (bytesRead < 0)
		return -1;

	bytesWritten = write_to_server(bytes);
	bytesRead = read_from_server(bytes);

	// Return path until the operation isnt complete
	if (status == 0) {
		if (clientOutput.find_header_end() == false) {
			if (clientOutput.size > 8000)	// TODO: Fix magic variable
				return -1;	// ERROR: CGI Header is too big
			return 0;	// Still no CGI Header
		}
		buildCgiHeader();
	}
	return write_to_client(bytes, events);
}
}
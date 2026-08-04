#pragma once

#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include "core.hpp"
#include "http/Request.hpp"

namespace HTTP {

template <usize bufferSize> inline
isize Request<bufferSize>::prepare_cgi() {

	open()
	// Open CGI here (invalid CGIs should be checked in parsing, not here)
	// Open pipes, fork, dup fds, execve
	// Set FDs
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
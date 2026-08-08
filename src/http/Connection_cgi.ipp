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

template <usize bufferSize> inline
isize Connection<bufferSize>::cgi_first_run() {
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
	fd.readEnd = fdOutput[0];
	fd.writeEnd = fdInput[1];
	cgiStartTime = g_timeNow;
}

// NOTES: i think the call here is because CGI output is server controlled, 
// we perform little error checking and presume the output is correct. 
// Only quick sanity checks
template <usize bufferSize> inline
isize Connection<bufferSize>::parse_cgi_line(Buffer<bufferSize> &src, Buffer<bufferSize> &dst) {
	char *field = src.data + src.start;
	char *value = src.data + src.start;
	char *end = src.data + src.end;
	const usize totalLength = src.end - src.start;

	isize type = s_match_field(value, end);
	if (type <= 0) {
		if (type < 0)
			status = Status::i500;
		return type;
	}

	if (type == Field::STATUS) {
		status = value;
		isize rvalue = status.is_valid() == true ? 0 : -1;
		if (rvalue == -1)
			status = Status::i500;	// CGI output an invalid status, should be server error
		const char *str = status.c_str();
		usize length = status.size();
		dst.insert(str, length, 256 - length);
		return rvalue;
	}
	else {
		dst.append(field, totalLength);
	}
	return type;
}

/*	Header is built in stack memory while parsing the header from client output
	When the header is built, it then appends part of the CGI body to tmp buffer
	up to how many bytes will fit in a single write */
template <usize bufferSize> inline
isize Connection<bufferSize>::build_cgi_header() {
	Buffer<2 * bufferSize> tmpBuffer;

	tmpBuffer.index = 256;
	tmpBuffer.size = 256;
	tmpBuffer.start = 256;

	while (clientOutput.find_line_end() == 1) {
		if (parse_cgi_line() == -1) {
			
		}
	}
}

/*	The pipe fds here are configured to be non-blocking and read/write errors are ignored
	Failure conditions for these fds are instead handled by CGI timeouts */
template <usize bufferSize> inline
isize Connection<bufferSize>::cgi_method(usize bytes, u32 events) {
	isize bytesRead, bytesWritten;

	bytesRead = read_from_client(bytes, events);
	if (bytesRead < 0)
		return -1;

	bytesWritten = write_to_server(bytes);
	bytesRead = read_from_server(bytes);

	isize delta = ((bytesWritten < 0 || bytesRead < 0) ? -1 : 1);
	bonusTime = CLAMP(bonusTime + delta, 0, 30);

	// Return path until the operation isnt complete
	if (status.is_set()) {
		if (clientOutput.find_header_end() == false) {
			if (clientOutput.is_full())
				return -1;	// ERROR: CGI Header is too big
			return 0;	// Still no CGI Header
		}
		build_cgi_header();
	}
	return write_to_client(bytes, events);
}
}

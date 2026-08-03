#pragma once
#include "Request.hpp"

// === DEL ==========================================================================
template <usize bufferSize> inline // Header will already be built in the configure function
isize HTTP::Request<bufferSize>::del_method(usize bytes, u32 events) {
	return write_to_client(bytes, events);
}

// === GET ==========================================================================
// Header will already be built in the configure function
template <usize bufferSize>
isize HTTP::Request<bufferSize>::get_method(usize bytes, u32 events) {
	isize bytesRead = read_from_server(bytes);
	if (bytesRead < 0)
		return bytesRead;
	return write_to_client(bytes, events);
}

// === POST =========================================================================
template <usize bufferSize> inline
isize HTTP::Request<bufferSize>::post_method(usize bytes, u32 events) {
	isize bytesRead = read_from_client(bytes, events);
	if (bytesRead < 0)
		return bytesRead;

	isize bytesWritten = write_to_server(bytes);
	if (bytesWritten < 0)
		return bytesWritten;

	// Return path until the operation isnt complete
	if (status == 0 && fd.writeEnd == -1) {
		// set status
		// build header
	}
	return write_to_client(bytes, events);	
}

/* === CGI ==========================================================================
The pipe fds here are configured to be non-blocking and read/write errors are ignored
Failure conditions for these fds are instead handled by CGI timeouts
*/
template <usize bufferSize> inline
isize HTTP::Request<bufferSize>::cgi_method(usize bytes, u32 events) {
	isize bytesRead, bytesWritten;

	bytesRead = read_from_client(bytes, events);
	if (bytesRead < 0)
		return -1;

	bytesWritten = write_to_server(bytes);
	bytesRead = read_from_server(bytes);

	// Return path until the operation isnt complete
	if (status == 0) {
		if (clientOutput.find_header_end(0) == false) {
			if (clientOutput.size > 8000)	// TODO: Fix magic variable
				return -1;	// ERROR: CGI Header is too big
			return 0;	// Still no CGI Header
		}
		buildCgiHeader();
	}
	return write_to_client(bytes, events);
}

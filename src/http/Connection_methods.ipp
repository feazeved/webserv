#pragma once
#include "Connection.hpp"

namespace HTTP {

template <usize bufferSize> inline
isize Connection<bufferSize>::get_first_run() {
		// Open files
		// Set FDs
}

template <usize bufferSize> inline
isize Connection<bufferSize>::del_first_run() {
		// Open files
		// Set FDs
}

template <usize bufferSize> inline
isize Connection<bufferSize>::post_first_run() {
		// Open files
		// Set FDs
}

// === DEL ==========================================================================
template <usize bufferSize> inline // Header will already be built in the configure function
isize Connection<bufferSize>::del_method(usize bytes, u32 events) {
	return write_to_client(bytes, events);
}

// === GET ==========================================================================
// Header will already be built in the configure function
template <usize bufferSize>
isize Connection<bufferSize>::get_method(usize bytes, u32 events) {
	isize bytesRead = read_from_server(bytes);
	if (bytesRead < 0)
		return bytesRead;
	return write_to_client(bytes, events);
}

// === POST =========================================================================
template <usize bufferSize> inline
isize Connection<bufferSize>::post_method(usize bytes, u32 events) {
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
}

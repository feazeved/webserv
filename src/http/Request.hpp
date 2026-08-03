#pragma once

#include <unistd.h>
#include <sys/epoll.h>

#include "core.hpp"
#include "Request_helpers.inl"
#include "http/Buffer.hpp"

namespace HTTP {

namespace Attributes {

enum Attributes {
	GET = 1 << 0,
	POST = 1 << 1,
	DELETE = 1 << 2,
	CGI = 1 << 3,
	HOST = 1 << 4,
	CHUNKED = 1 << 5,
	DONE = 1 << 7

};

}

typedef struct {
	struct {
		u32	index;
		u32 size;
	}	path, query, cookie;
}	RequestVars;

template <usize bufferSize>
class Request {
public:
	RequestVars vars;
	usize bodySize, chunkSize;
	Buffer<bufferSize> clientInput, clientOutput;
	struct {
		i32 client;		// Duplex FD
		i32 writeEnd;	// CGI Input or POST
		i32 readEnd;	// CGI Output or GET/DEL
	} fd;

	union {
		u64 state;
		struct {
			u32 metadata;
			u16 status;
			u8 info;
			u8 type;
		};
	};

// Methods
isize configure();
void buildHeader();
void buildCgiHeader() {
	// parse CGI header
	// build header based on this parsing
	// set status
	// Setting the status means the client output buffer was filled
}

// Header will already be built in the configure function
isize del_method(usize bytes, u32 events) {
	return write_to_client(bytes, events);
}

isize get_method(usize bytes, u32 events) {
	isize bytesRead;
	if (fd.readEnd >= 0) {
		bytesRead = clientOutput.read(fd.readEnd, bytes);	// TODO: Need to also check if CGI signalled EOF
		if (bytesRead < 0)
			return bytesRead;
	}

	if (status == 0 && bytesRead == 0) {
		close(fd.readEnd);
		fd.readEnd = -1;
		// set status
		// build header
	}
	return write_to_client(bytes, events);
}

isize post_method(usize bytes, u32 events) {
	isize bytesRead = read_from_client(bytes, events);
	if (bytesRead < 0)
		return bytesRead;

	isize bytesWritten = write_to_server(bytes);
	if (bytesWritten < 0)
		return bytesWritten;

	bytesRead = read_from_server(bytes);
	if (bytesRead < 0)
		return bytesRead;

	// Return path until the operation isnt complete
	if (status == 0 && fd.writeEnd == -1) {
		// set status
		// build header
	}
	return write_to_client(bytes, events);	
}

isize cgi_method(usize bytes, u32 events) {
	isize bytesRead = read_from_client(bytes, events);
	if (bytesRead < 0)
		return bytesRead;

	isize bytesWritten = write_to_server(bytes);
	if (bytesWritten < 0)
		return bytesWritten;

	bytesRead = read_from_server(bytes);
	if (bytesRead < 0)
		return bytesRead;

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

isize read_from_server(usize bytes) {
	isize bytesRead = clientOutput.read(fd.readEnd, bytes);
	if (bytesRead < 0)
		return bytesRead;
	if (bytesRead == 0) {
		close(fd.readEnd);
		fd.readEnd = -1;
	}
	return bytesRead;
}

isize write_to_server(usize bytes) {
	isize bytesWritten;

	if (type & HTTP::Attributes::CHUNKED)
		bytesWritten = dechunk(bytes, clientOutput);
	else 
		bytesWritten = clientOutput.write(fd.writeEnd, bodySize);
	if (bytesWritten < 0)
		return bytesWritten;

	bodySize -= bytesWritten;
	if (bodySize == 0) {
		close(fd.writeEnd);
		fd.writeEnd = -1;	// Finished reading
	}
	return bytesWritten;
}

// Common to all
isize write_to_client(usize bytes, usize events) {
	isize bytesWritten = clientOutput.write(fd.client, bytes);
	if (bytesWritten < 0)
		return bytesWritten;	// TODO: tmp error path
	return bytesWritten;
}

// Common to POST and CGI
isize read_from_client(usize bytes, u32 events) {
	isize bytesRead = clientOutput.read(fd.client, bytes);
	if (bytesRead < 0)
		return bytesRead;
	return bytesRead;
}

// This function dechunks from a source buffer to a stack buffer, then writes from this stack buffer
// Any bytes that weren't consumed by the write are copied back to the start of the source buffer, 
// effectively performing compaction.
isize dechunk(usize bytes, Buffer<bufferSize>& src) {
	Buffer<bufferSize> tmpBuffer;

	while (src.index < src.size) {
		if (chunkSize == SIZE_MAX) {
			if (src.find_line_end(2) == false)
				break;
			chunkSize = s_strtol16(src.data + src.start);
			if (chunkSize > bodySize)
				return -4;	// CLOSING ERROR: Body size was greater than maximum allowed
			bodySize -= chunkSize;	// bodySize here represents the total allowed
			if (chunkSize == 0)
				bodySize = 0;		// Signals end of message
			src.index += 2;
			break;
		}
		else if (chunkSize > 0) {
			usize bytesAppended = tmpBuffer.append(src, chunkSize);
			chunkSize -= bytesAppended;	// Guaranteed to be chunksize or less
		}
		else {
			if (MEMCMP(src.data + src.index, "\r\n", 2) != 0)
				return -2;	// CLOSING ERROR: Chunk size was 0 and it did not have \r\n
			src.index += 2;
			break;
		}
	}

	if (tmpBuffer.size == 0)
		return 0;				// CHECK: Nothing was appended

	isize bytesWritten = tmpBuffer.write(fd.writeEnd, bytes);
	if (bytesWritten < 0)
		return bytesWritten;	// FATAL ERROR: Write error

	// Optimization opportunity here to have src copy directly to itself
	// TODO: The buffer is only going to fill with data related to the chunks, decide if compaction is worth given current length
	// Otherwise just prepend the remainder to the end of what was read
	if (src.index < src.size) { 
		isize bytesRemaining = src.size - src.index;
		tmpBuffer.append(src, (usize)bytesRemaining);
	}
	if (tmpBuffer.size > tmpBuffer.index) {
		src.reset();
		src.append(tmpBuffer, SIZE_MAX);
	}

	return bytesWritten;
}

// ======== Constructors ====================
Request() :
	type(0),
	bodySize(SIZE_MAX) {
	}
};
}

#pragma once

#include "core.hpp"
#include <unistd.h>
#include <sys/epoll.h>
#include "Request_helpers.hpp"

// Implement a compact function so that if necessary, moves cookie to the end of query so that path + query + cookie < 8192

// Implement state actions to Request so that it switches between reading and writing seamlessly
	// It needs to set the epoll variables and confirm upon entry that it can write
	// With chunked transfer encoding, it will constantly switch between read and write

// Implement method actions to Request so that it can GET, POST, DELETE, CGI or ERROR
	// This involves returning a HTTP Header with the appropriate status code and the payload


namespace HTTP {

namespace Attributes {

enum Attributes {
	METHOD_GET = 1 << 0,
	METHOD_POST = 1 << 1,
	METHOD_DELETE = 1 << 2,
	CGI = 1 << 3,
	VERSION = 1 << 4,
	HOST = 1 << 5,
	CHUNKED = 1 << 6,
	FOO = 1 << 7
};

enum State {
	READING = 1 << 0,		// Reading header
	PROCESSING = 1 << 1,	// Reading body
	WRITING = 1 << 2		// Writing
};
}

typedef struct {
	struct
	{
		u32	index;
		u32 size;
	}	path, query, cookie;
}	RequestVars;

// Class has a read call that consumes lines
// Possible states:
// Reading: Reading Header, Reading Body
// Writing: 
// Limits: ~8kb per line, ~4kb for the target
class Request {
public:
	i32 fd;
	u8 buffer[8000];
	u8 type; // bitfield: (-) (-) (CHUNKED) (HOST) (HTTP VERSION) (DELETE) (POST) (GET)
	u8 state;
	bool syscalled;
	u32 readOffset, curOffset, writeOffset, line_count;
	RequestVars vars;
	u64 requestSize;


// Methods
// 0) No reads, 1) Read, -1) Failed Reading, -2) Line is too big
i8 read(usize bytes, u32 events) {
	if ((events & EPOLLIN) == 0)
		return -1;		// ERROR: Attempted to read but epoll was not set or ready

	if (curOffset + 1 < writeOffset)
		return 0;
	if (writeOffset + bytes > sizeof(buffer))
		return -1;	// ERROR: Line is too long

	isize bytesRead = ::read(fd, buffer + writeOffset, bytes);
	if (bytesRead < 0)
		return -2;	// TODO: Need to distinguish between first read fail and other read fails

	writeOffset += (usize) bytesRead;	// TODO: what do we do on failures?
	return 1;
}

i8 write(usize bytes, u32 events)
{
	if ((events & EPOLLOUT) == 0)
		return -1;		// ERROR: Attempted to write but epoll was not set or ready

	if (curOffset + 1 < writeOffset)
		return 0;	// Nothing to write, should be an error if the payload isnt 0

	bytes = MIN(bytes, writeOffset - curOffset);
	isize bytesWritten = ::write(fd, buffer + curOffset, bytes);	// TODO: The write here reads from a different FD, and from a different buffer too
	if (bytesWritten < 0)
		return -1;
	return 1;
}

// Reading state for the header, returns true when finished parsing the header
i8 parse_header(usize bytes, u32 events) {
	i8 state = read(bytes, events);
	if (state < 0)
		return -1;	// ERROR: Failed reading

	while (curOffset < writeOffset - 1) {
		if (buffer[curOffset] == '\r' && buffer[curOffset + 1] == '\n')
		{
			parseLine(curOffset - readOffset);	// TODO: check for errors
			curOffset += 2;
			line_count++;
			if (curOffset < writeOffset - 1 && buffer[curOffset] == '\r' && buffer[curOffset + 1] == '\n') {
				state = HTTP::Attributes::PROCESSING;
				curOffset += 2;
				readOffset = curOffset;
				return 1;
			}
			readOffset = curOffset;
		}
		else
			curOffset++;
	}
	return 0;	// Actually should return something more useful like request status (processing, etc)
}

// Reading state for the body
i8 parse_body(usize bytes, u32 events)
{
	if (read(bytes, events) < 0)
		return -1;	// ERROR: Failed reading

}

isize exec(usize bytes, u32 events)
{

}

void close() {
	// close operations
	fd = -1;
	type = 0;
	line_count = 0;
	readOffset = 0;
	curOffset = 0;
	writeOffset = 0;
}

i32 parseTarget(const char *str, const char *end);
i32 parseFirstLine(usize length);
i32 parseLine(usize length);

// ======== Constructors ====================
Request() :
	fd(-1),
	buffer(),
	type(0),
	state(0),
	readOffset(0),
	curOffset(0),
	writeOffset(0),
	line_count(0),
	requestSize(SIZE_MAX) {
	}
};
}

#include "Request_parse.hpp"
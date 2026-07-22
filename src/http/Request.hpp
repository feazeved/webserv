#pragma once

#include "core.hpp"
#include <unistd.h>
#include <sys/epoll.h>
#include "Request_helpers.hpp"

namespace HTTP {

enum RequestAction {
	REQ_CONTINUE,
	REQ_CLOSE,
	REQ_WRITE
};

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
class Request
{
public:
	u8 buffer[32 * 1024];
	u8 type;	// bitfield: (CHUNKED) () () (HOST) (HTTP VERSION) (DELETE) (POST) (GET)
	u32 readOffset, curOffset, writeOffset;
	u32 line_count;
	RequestVars vars;
	u64 requestSize;
	i32 fd;

// Methods
isize read(usize bytes)
{
	if (writeOffset + bytes > sizeof(buffer))
		return -1;	// ERROR: Line is too long

	isize bytesRead = ::read(fd, buffer + writeOffset, bytes);	// TODO: check for errors
	if (bytesRead <= 0)
		return bytesRead;

	writeOffset += (usize) bytesRead;
	while (curOffset < writeOffset - 1)
	{
		if (buffer[curOffset] == '\r' && buffer[curOffset + 1] == '\n')
		{
			// if (curOffset + 3 <= writeOffset && str[curOffset + 2] == '\r' && str[curOffset + 3] == '\n')
			// 	header_done;
			parseLine(curOffset - readOffset);	// TODO: check for errors
			curOffset += 2;
			readOffset = curOffset;
			line_count++;
		}
		else
			curOffset++;
	}
	return bytesRead;	// Actually should return something more useful like request status (processing, etc)
}

void close()
{
	// close operations
	fd = -1;

}

// its called by ServerManager.run().
// return values:
// 		REQ_CONTINUE: response is not ready.
// 		REQ_CLOSE:	  close the connection (will remove fd from epoll)
// 		REQ_WRITE:	  says you're ready to write the response and ServerManager will modify its epoll so that you write
//
// TODO: check when to return REQ_CLOSE. Maybe should check for events & (EPOLLHUP | EPOLLERR) to close the connection properly.
// Maybe should close according to something in the request? Maybe need to close if EPOLLIN && (rvalue = read()) == 0
RequestAction	handleEvent(u32 events) {
	if (events & EPOLLIN) {
		isize	n = read(4096);
		if (n <= 0) {
			return (REQ_CLOSE);
		}

		// if ready to write
		// return (REQ_WRITE);
	}
	if (events & EPOLLOUT) {
		// write  response
	}
	return (REQ_CONTINUE);
}

i32 parseTarget(const char *str, const char *end);
i32 parseFirstLine(usize length);
i32 parseLine(usize length);

// ======== Constructors ====================
Request() :
	buffer(),
	type(0),
	readOffset(0),
	curOffset(0),
	writeOffset(0),
	line_count(0),
	requestSize(SIZE_MAX),
	fd(-1)
{
}
};
}

#include "Request_parse.hpp"
#pragma once

#include "core.hpp"
#include "Buffer.hpp"
#include <cctype>
#include <cstring>
#include <sys/epoll.h>
#include "Request_helpers.hpp"

namespace HTTP {

enum RequestAction {
	REQ_CONTINUE,
	REQ_CLOSE,
	REQ_WRITE
};

// Class has a read call that consumes lines
class Request
{
public:
	Buffer buffer;
	u8 type;	// bitfield: (CHUNKED) () () () (HTTP VERSION) (DELETE) (POST) (GET)
	u32 readOffset, curOffset, writeOffset;
	u32 pathOffset, queryOffset;
	usize pathSize, querySize;
	u32 line_count;
	u64 requestSize;
	i32 fd;

// Methods
isize read(usize bytes)
{
	isize bytesRead = buffer.read(fd, bytes);	// TODO: check for errors
	const char *str = (const char*) buffer.data;

	if (bytesRead <= 0)
		return bytesRead;
	writeOffset += (usize) bytesRead;
	while (curOffset < writeOffset - 1)
	{
		if (str[curOffset] == '\r' && str[curOffset + 1] == '\n')
		{
			// if (curOffset + 3 <= writeOffset && str[curOffset + 2] == '\r' && str[curOffset + 3] == '\n')
			// 	header_done;
			parseLine(curOffset - readOffset);	// TODO: check for errors
			curOffset += 2;
			readOffset = curOffset;
			line_count++;
		}
	}
	return bytesRead;	// Actually should return something more useful like request status (processing, etc)
}

// its called by ServerManager.run().
// return values:
// 		REQ_CONTINUE: response is not ready.
// 		REQ_CLOSE:	  close the connection (will remove fd from epoll)
// 		REQ_WRITE:	  says you're ready to write the response and ServerManager will modify its epoll so that you write
//
// TODO: check when to return REQ_CLOSE. Maybe should check for events & (EPOLLHUP | EPOLLERR) to close the connection properly.
// Maybe should close according to something in the request? Maybe need to close if EPOLLIN && (rvalue = read()) == 0
HTTP::RequestAction	handleEvent(u32 events) {
	if (events & EPOLLIN) {
		isize	n = read(BUFFER_CAPACITY);
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

// changed this to compile
i32 parseHost(const char *str, const char *end) {
	(void)str, (void)end;
	return (0);
} // Host doesnt need to be stored if its resolved immediately

i32 parseFirstLine(usize length);
i32	parseTarget(const char* str, const char* end);
i32 parseLine(usize length);

// ======== Constructors ====================
Request(i32 fd) :
	buffer(),
	type(0),
	readOffset(0),
	curOffset(0),
	writeOffset(0),
	pathOffset(0),
	queryOffset(0),
	pathSize(0),
	querySize(0),
	line_count(0),
	requestSize(SIZE_MAX),
	fd(fd)
{
}

private:
	Request();
};
#include "Request_parse.hpp"
}

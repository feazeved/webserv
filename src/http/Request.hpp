#pragma once

#include "core.hpp"
#include "Buffer.hpp"
#include "Request_helpers.hpp"

namespace HTTP {

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

i32 parseTarget(const char *str, const char *end);
i32 parseHost(const char *str, const char *end); // Host doesnt need to be stored if its resolved immediately
i32 parseFirstLine(usize length);
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

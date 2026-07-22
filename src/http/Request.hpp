#pragma once

#include "core.hpp"
#include "Buffer.hpp"
#include <cctype>
#include <cstring>
#include <stdexcept>
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

// Split path/query at the first ?. V
// Reject whitespace and control characters. V
// Reject malformed encodings such as %, %2, or %GG. V
// Reject decoded NUL bytes such as %00. V
// Normalize or reject . and .. before filesystem access.
// Ensure the final resolved filesystem path remains inside the configured root.
// Do not blindly convert %2F into /; that can change path structure.
// Apply a request-target length limit and return 414 URI Too Long when exceeded
// Needs to validate the target based on above reqs
i32 parseTarget(const char *str, const char *end){
    if (str >= end)
        return -1;

    const char *p = str;
    const char *questionMark = NULL;

    while (p < end)
    {
        if(*p <= 32)
            return -1;
        if(*p == '?')
        {
            if(!questionMark)
                questionMark = p;
            else
                return -1;
        }
        if(*p == '%' && p + 2 < end)
        {
            if(!std::isxdigit(*(p+1)) || !std::isxdigit(*(p+2))
                ||(*(p+1) == '0' && *(p+2) == '0'))
                return -1;
        }
        p++;
    }
    pathOffset = str - (const char *)buffer.data;
    if (questionMark)
    {
        pathSize = questionMark - str;
        queryOffset = (questionMark + 1) - (const char *)buffer.data;
        querySize = end - (questionMark + 1);
    }
    else
        pathSize = end - str;
    return 0;
}

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

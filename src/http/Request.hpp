#pragma once

#include <unistd.h>
#include <sys/epoll.h>

#include "core.hpp"
#include "Request_helpers.hpp"
#include "http/Buffer.hpp"

// Implement a compact function so that if necessary, moves cookie to the end of query so that path + query + cookie < 8192

// Implement state actions to Request so that it switches between reading and writing seamlessly
	// It needs to set the epoll variables and confirm upon entry that it can write
	// With chunked transfer encoding, it will constantly switch between read and write

// Implement method actions to Request so that it can GET, POST, DELETE, CGI or ERROR
	// This involves returning a HTTP Header with the appropriate status code and the payload

// Create a separate Parser class that handles the reading of the header only
	// It is supposed to output a struct containing all of the relevant metadata
	// It does not own the buffer

// Whenever status is deduced, it already loads the string into output buffer index 9 e.g. (HTTP/1.1 _)


namespace HTTP {

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
	Buffer<8192> input, output;
	u8 type; // bitfield: (-) (-) (-) (CHUNKED) (HOST) (DELETE) (POST) (GET)
	u8 state;	// TODO: transfer all of these to a metadata struct
	u32 status;
	u32 lineIndex, lineCount;
	RequestVars vars;
	usize requestSize;

// (Reentrant) Reading state for the header, returns true when finished parsing the header
i8 parse_header(usize bytes, u32 events) {
	i8 rvalue = input.read(fd, bytes, events);
	if (rvalue < 0)
		return -1;	// ERROR: Failed reading

	bool header_done = false;
	u32 lineEnd = input.find_line_end(header_done);
	if (lineEnd == UINT32_MAX)	// Not done reading a line
		return 0;

	const char *ptr = (const char *)input.data;
	if (header_done)
		state = HTTP::Attributes::PROCESSING;

	if (parseLine(ptr + lineIndex, ptr + lineEnd, lineCount) != 0)
		return -1; // TODO: This also determines if the parsing is done, given errors exist
	lineCount++;
	lineIndex = lineEnd;
	return 0;	// Actually should return something more useful like request status (processing, etc)
}

// Reading state for the body
// Here we are reading into the input buffer, but the newline requirement only applies to the content length
// The body may well be over 8192 bytes, therefore it needs to be streamed appropriately
i8 parse_body(usize bytes, u32 events) {
	// if (input.read(bytes, events) < 0)
	// 	return -1;	// ERROR: Failed reading

	// if ()
}

void close() {
	// close operations
	type = 0;
	input.clear();
	output.readOffset = 0;
	output.writeOffset = 9;
}

i32 parseTarget(const char *str, const char *end);
i32 parseFirstLine(const char *str, const char *end);
i32 parseLine(const char *str, const char *end, u32 lineCount);
void buildHeader();

// ======== Constructors ====================
Request() :
	fd(-1),
	input(),	// TODO: empty constructors for buffer
	output(),
	type(0),
	state(0),
	requestSize(SIZE_MAX) {
		output.readOffset = 0;
		output.writeOffset = 0;
		output.append("HTTP/1.1 ");
	}
};
}

#include "Request_parse.hpp"
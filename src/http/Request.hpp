#pragma once

#include <cstdint>
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

// Fake envp added to config for easy insertion

namespace HTTP {

typedef struct {
	struct
	{
		u32	index;
		u32 size;
	}	path, query, cookie;
}	RequestVars;

namespace Attributes {

enum Attributes {
	METHOD_GET = 1 << 0,
	METHOD_POST = 1 << 1,
	METHOD_DELETE = 1 << 2,
	CGI = 1 << 3,
	HOST = 1 << 4,
	CHUNKED = 1 << 5,
	DONE = 1 << 7
};

enum State {
	READING = 1 << 0,		// Reading header
	PROCESSING = 1 << 1,	// Reading body
	WRITING = 1 << 2,		// Writing
	SKIPPING = 1 << 3,		// Skipping until new header

	PROCESSING_LENGTH = 1 << 6, // Reading the header for the body
	FIRST_LINE = 1 << 7
};
}

// Class has a read call that consumes lines
// Possible states:
// Reading: Reading Header, Reading Body
// Writing: 
// Limits: ~8kb per line, ~4kb for the target
class Request {
	static const usize metadataSize = sizeof(i32) + 2 * sizeof(u8) + 4 * sizeof(u32) + sizeof(RequestVars) + sizeof(usize);

public:
	i32 fd;
	Buffer<16 * 1024 - metadataSize> input, output;
	u8 type; // bitfield: (DONE) (-) (CHUNKED) (HOST) (CGI) (DELETE) (POST) (GET)
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

	u32 lineEnd;
	while ((lineEnd = input.find_line_end()) != UINT32_MAX) {
		char *ptr = (char *) input.data;
		parseLine(ptr + lineIndex, ptr + lineEnd, lineCount);
		lineCount++;
		lineIndex = lineEnd;
		if (type & HTTP::Attributes::DONE)
			return prepare();
	}
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

i8 upload(usize bytes, u32 events);

void close() {
	// close operations
	if (fd > 0) {
		::close(fd);
		fd = -1;
	}

	type = 0;
	input.clear();
	output.cursor = 0;
	output.size = 9;
}

i32 parseTarget(char *str, char *end);
i32 parseFirstLine(char *str, char *end);
i32 parseLine(char *str, char *end, u32 lineCount);

void buildHeader();
i32 prepare();

// ======== Constructors ====================
Request() :
	fd(-1),
	input(),	// TODO: empty constructors for buffer
	output(),
	type(0),
	state(0),
	requestSize(SIZE_MAX) {
		output.cursor = 0;
		output.size = 0;
	}
};
}

#include "Request_parse.hpp"
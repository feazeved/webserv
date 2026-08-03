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

// Parsing
isize parse_header(usize bytes, u32 events);
isize parse_first_line(char *str, char *end);
isize parse_line(char *str, char *end);
isize parse_target(char *str, char *end);

// Configuration
isize configure();
void buildHeader();
void buildCgiHeader();

// HTTP Methods
isize del_method(usize bytes, u32 events);
isize get_method(usize bytes, u32 events);
isize post_method(usize bytes, u32 events);
isize cgi_method(usize bytes, u32 events);

// Common
isize read_from_server(usize bytes);
isize write_to_server(usize bytes);
isize write_to_client(usize bytes, u32 events);
isize read_from_client(usize bytes, u32 events);
isize dechunk(usize bytes, Buffer<bufferSize>& src);

// This function dechunks from a source buffer to a stack buffer, then writes from this stack buffer
// Any bytes that weren't consumed by the write are copied back to the start of the source buffer, 
// effectively performing compaction.

// ======== Constructors ====================
Request() :
	type(0),
	bodySize(SIZE_MAX) {
	}
};
}

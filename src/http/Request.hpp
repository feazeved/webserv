#pragma once
#include <string>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>

#include "core.hpp"
#include "Status.hpp"
#include "Buffer.hpp"
#include "HTTP.hpp"

namespace HTTP {

#define REQUEST_INL(ret_type) template <usize bufferSize> ret_type inline HTTP::Request<bufferSize>::

template <usize bufferSize>
class Request {
public:
	struct Span16 {
		u16 index;
		u16 size;
	}	path, query, cookie;

	usize bodySize, chunkSize;
	Status status;
	u8 mode;
	u8 options;
	u8 contentType;

	// Parsing
	isize check_location(Buffer<bufferSize> &src, ServerConfig* cfg);
	isize parse_target(Buffer<bufferSize> &src, ServerConfig* cfg);
	isize parse_first_line(Buffer<bufferSize> &src, ServerConfig* cfg);
	isize parse_header(Buffer<bufferSize> &src);
	isize parse_line(Buffer<bufferSize> &src);
	isize parse_cgi_line(Buffer<bufferSize> &src, Buffer<bufferSize> &dst);
};
}

#include "Request_first.ipp"
#include "Request_parse.ipp"
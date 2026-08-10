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

namespace Attributes {
	enum e_http_attributes {
		GET = 1 << 0,
		POST = 1 << 1,
		DELETE = 1 << 2,
		CGI = 1 << 3,
		HOST = 1 << 4,
		CHUNKED = 1 << 5,
		DONE = 1 << 7
	};
}

namespace Field {
	enum e_http_field
	{
		INVALID = -1,
		UNKNOWN = 0,
		STATUS = 1,
		LOCATION = 2,
		TRANSFER_ENCODING = 3,
		CONTENT_LENGTH = 4,
		CONTENT_TYPE = 5,
		HOST = 6,
		CONNECTION = 7
	};
}

struct Span16 {
	u16 index;
	u16 size;
};

template <usize bufferSize>
class Request {
public:
	Span16 path, query, cookie;

	usize bodySize, chunkSize;
	Status status;
	u8 info, type;
	u8 contentType;

	// Parsing
	isize parse_target(Buffer<bufferSize> &src, ServerConfig* cfg);
	isize parse_first_line(Buffer<bufferSize> &src, ServerConfig* cfg);

	isize parse_header(Buffer<bufferSize> &src);
	isize parse_line(Buffer<bufferSize> &src);
	isize parse_cgi_line(Buffer<bufferSize> &src, Buffer<bufferSize> &dst);
};
}

#include "Request_first.ipp"
#include "Request_parse.ipp"
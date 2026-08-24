#pragma once
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>

#include "core.hpp"
#include "Status.hpp"
#include "Buffer.hpp"
#include "HTTP.hpp"
#include "VirtualServer.hpp"

namespace HTTP {

#define REQUEST_INL(ret_type) ret_type inline HTTP::Request::

class Request {
public:

	// TODO: keeping track of size might not be necessary
	struct Span16 {
		u16 index;
		u16 size;
	}	path, query, cookies;

	u16 locationIndex;
	usize bodySize, chunkSize;
	Status status;
	u8 options;
	u8 contentType;

	void reset() {
		path.index = 0;
		path.size = 0;
		query.index = 0;
		query.size = 0;
		cookies.index = 0;
		cookies.size = 0;
		locationIndex = 0;
		bodySize = 0;
		chunkSize = 0;
		status.reset();
		options = 0;
		contentType = Mime::OCTET_STREAM;
	}

	// Parsing
	isize parse_target(HTTP_Buffer &src, VirtualServer* cfg);
	isize parse_first_line(HTTP_Buffer &src, VirtualServer* cfg, usize lineLength);
	isize parse_line(HTTP_Buffer &src, VirtualServer* cfg, usize lineLength);
	isize parse_cgi_line(HTTP_Buffer &src, HTTP_Buffer &dst);
	Mode::e_http_mode validate_header(HTTP_Buffer &src, VirtualServer* cfg);

};
}

#include "Request_first.ipp"
#include "Request_parse.ipp"
#include "Request_validate.ipp"

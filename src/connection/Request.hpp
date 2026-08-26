#pragma once
#include "core.hpp"
#include "Status.hpp"
#include "Buffer.hpp"
#include "VirtualServer.hpp"
#include "Span.hpp"

#define REQUEST_INL(ret_type) ret_type inline Request::

class Request {
public:
	Span path, query, cookies, interpreter;
	Span contentTypeHeader, contentSize;
	Location* location;
	usize bodySize, chunkSize;
	Status status;
	u8 options;
	u8 contentType;

	void reset() {
		MEMSET_INLINE(this, 0, sizeof(*this));
		status.reset();
		// contentType = Mime::OCTET_STREAM;
	}

	// Parsing
	isize validate_target(HTTP_Buffer &src, VirtualServer* cfg);
	isize parse_first_line(HTTP_Buffer &src, VirtualServer* cfg, usize lineLength);
	isize parse_line(HTTP_Buffer &src, VirtualServer* cfg, usize lineLength);
	isize parse_cgi_line(HTTP_Buffer &src, HTTP_Buffer &dst);
	Mode::e_http_mode validate_header(HTTP_Buffer &src, VirtualServer* cfg);

};

#include "Request_parse.ipp"
#include "Request_validate.ipp"

#pragma once
#include "core.hpp"
#include "Status.hpp"
#include "Buffer.hpp"
#include "VirtualServer.hpp"
#include "Span.hpp"

#define REQUEST_INL(ret_type) ret_type inline Request::

class Request {
public:

	// TODO: keeping track of size might not be necessary
	Span16 path, query, cookies, interpreter;

	u16 locationIndex;
	usize bodySize, chunkSize;
	Status status;
	u8 options;
	u8 contentType;

	void reset() {
		path.index = 0;
		path.length = 0;
		query.index = 0;
		path.length = 0;
		cookies.index = 0;
		path.length = 0;
		locationIndex = 0;
		bodySize = 0;
		chunkSize = 0;
		status.reset();
		options = 0;
		contentType = Mime::OCTET_STREAM;
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

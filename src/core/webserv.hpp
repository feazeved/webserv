#pragma once
#include "Array.hpp"
#include "Span.hpp"
#include "Status.hpp"

// These are exclusive states
namespace Mode {
	enum e_http_mode {
		PARSE_FIRST = 0,	// Changes to PARSE after first line
		PARSE,				// Calls setup when finished

		GET,				// Changes to FLUSH upon bodysize == 0
		AUTOINDEX,			// Changes to FLUSH upon entry == NULL

		POST_FIXED,			// Reads from client, changes to FLUSH upon bodySize == 0
		POST_CHUNKED,		// Reads from client, changes to POST_FIXED on chunk termination

		CGI,				// Reads from CGI, writes to client, changes to FLUSH upon EOF
		CGI_FIXED,			// Reads from client, changes to CGI upon bodySize == 0
		CGI_CHUNKED,		// Reads from client, changes to CGI_FIXED on chunk termination

		FLUSH				// Changes to FIRST_PARSE if keepalive is on. else terminates
	};
}

namespace Options {
	enum e_http_options {
		GET = 1 << 0,
		POST = 1 << 1,
		DELETE = 1 << 2,
		CGI = 1 << 3,
		SSE = 1 << 4,
		CHUNKED_LENGTH = 1 << 5,
		FIXED_LENGTH = 1 << 6,
		HOST = 1 << 7,
		KEEP_ALIVE = 1 << 8
	};
}

#define FIELD_TABLE {"status", "location", "transfer-encoding", \
	"content-length", "content-type", "host", "connection", \
	"accept", "cookie"}

namespace Field {
	enum e_http_field {
		UNKNOWN = 0,
		STATUS,
		LOCATION,
		TRANSFER_ENCODING,
		CONTENT_LENGTH,
		CONTENT_TYPE,
		HOST,
		CONNECTION,
		ACCEPT,
		COOKIES,
		COUNT
	};
}

#define MIME_TABLE {"html", "htm", "css", "json", "js", "png", "jpg", "jpeg", "gif", "txt"}

#define MIME_STRINGS {"\x18" "application/octet-stream", "\x09" "text/html", "\x09" "text/html",\
	"\x08" "text/css", "\x10" "application/json", "\x16" "application/javascript",\
	"\x09" "image/png", "\x0A" "image/jpeg", "\x0A" "image/jpeg", \
	"\x09" "image/gif", "\x0A" "text/plain"}

namespace Mime {
	enum e_http_mime_type {
		OCTET_STREAM = 0,
		HTML,
		HTM,
		CSS,
		JSON,
		JS,
		PNG,
		JPG,
		JPEG,
		GIF,
		TXT,
		COUNT
	};
}

#pragma once

#include <unistd.h>

#include "core.hpp"
#include "config.hpp"
#include "Arena.hpp"
#include "Array.hpp"
#include "StringView.hpp"
#include "Status.hpp"

namespace HTTP {

struct Token {
	enum Type {
		OPEN_BRACKET,
		CLOSE_BRACKET,
		SEMICOLON,
		WORD
	}	type;
	StringView32 value;
};

struct Directive {
	StringView32 name;
	Array32<StringView32> args;
};

struct Location {
	StringView32 url;
	StringView32 root;
	StringView32 index;			// If this is specified, its a file
	StringView32 uploadStore;
	StringView32 cgiBlock;		// Non-empty block range, including braces
	StringView32 redirectTarget;
	Status redirectStatus;
	u8 methods;
	bool autoindex;

	Location() : methods(0), autoindex(false) {}
};

// Switch (no read, read, read chunked, can read)

// These are exclusive states
namespace Mode {
	enum e_http_mode {
		PARSING = 0,
		GET = 1 << 0,
		POST = 1 << 1,
		FLUSHING = 1 << 2,	// Is done, streaming header
		CGI = 1 << 3,
		SSE = 1 << 4,
		CLOSE = 1 << 5		// Also done, streaming header, will close
	};
}

// namespace State {
// 	enum e_http_state {
// 		READING_FROM_CLIENT = 1,
// 		WRITING_TO_CLIENT = 2,
// 		READING_FROM_SERVER = 4,
// 		WRITING_TO_SERVER = 8
// 	};
// }

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
		CONNECTION_TYPE = 1 << 8	// Keep alive or
	};
}

#define FIELD_TABLE {"status", "location", "transfer-encoding", \
	"content-length", "content-type", "host", "connection", \
	"accept", "cookie"}	// TODO: add sse

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

#define MIME_TABLE {"html", "htm", "css", "json", \
	"js", "png", "jpg", "jpeg", "gif", "txt"}

#define MIME_STRINGS {"\x18" "application/octet-stream", "\x09" "text/html", "\x09" "text/html",\
	"\x08" "text/css", "\x10" "application/json", "\x16" "application/javascript",\
	"\x09" "image/png", "\x0A" "image/jpeg", "\x0A" "image/jpeg", \
	"\x09" "image/gif", "\x0A" "text/plain"}

namespace Mime {
	enum e_mime_type {
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
}

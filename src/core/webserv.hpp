#pragma once
#include "Array.hpp"
#include "StringView.hpp"
#include "Status.hpp"

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

// TODO: create a Location struct inside VirtualServers with specific logic
// TODO: store 2 byte starts for each path here, then MEMCMP 12 bytes to get indexes that match, and only compare those indices
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

// These are exclusive states
namespace Mode {
	enum e_http_mode {
		PARSE = 0,
		GET = 1 << 0,
		POST = 1 << 1,
		FLUSH = 1 << 2,	// Streams header, then parses again
		CGI = 1 << 3,
		SSE = 1 << 4,
		CLOSE = 1 << 5		// Streams header, then closes
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
		CONNECTION_TYPE = 1 << 8	// Keep alive or
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

enum e_ascii {
	ASCII_DIGITS      = 9,   // value <= digits
	ASCII_HEX         = 15,  // value <= hex
	ASCII_LETTERS     = 35,  // A-Z / a-z map to 10-35
	ASCII_IDENT       = 36,  // _
	ASCII_RFC_SYMBOLS = 37,  // RFC 3986 path symbols
	ASCII_SYMBOLS     = 38,  // other symbols
	ASCII_SPACE       = 39,
	ASCII_CONTROL     = 40,
	ASCII_INVALID     = 255
};

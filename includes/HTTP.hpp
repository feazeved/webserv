#pragma once

#include <unistd.h>
#include "core.hpp"
#include "Array.hpp"
#include "StringView.hpp"
#include "Status.hpp"
#include "State.hpp"

namespace HTTP {

#define MAX_VIRTUAL_SERVERS 64

struct Location {
	StringView path;
	StringView root;
	StringView index;
	StringView uploadStore;

	StringView cgiExtension;
	StringView cgiInterpreter;
	StringView redirectTarget;
	Status redirectStatus;
	u8 methods;
	bool autoindex;

	Location() : methods(0), autoindex(false) {}
};

struct ServerConfig {
	// std::map<long, StringView>	errors;
	StringView					clientErrors[32];
	// StringView				serverErrors[12];
	Array<Location>				locations;
	StringView					host;
	usize						port;
	usize						maxBodySize;
	Game::State					*gameState;

	ServerConfig() : port(SIZE_MAX), maxBodySize(SIZE_MAX), gameState(NULL) {}
};

#define HTTP_BUFFERSIZE 16384

#ifdef PIPE_BUF
	#if PIPE_BUF > 4096
		#define ATOMIC_IOSIZE 4096
	#else
		#define ATOMIC_IOSIZE PIPE_BUF
	#endif
#else
	#ifdef _POSIX_PIPE_BUF
		#define ATOMIC_IOSIZE _POSIX_PIPE_BUF
	#else
		#define ATOMIC_IOSIZE 512
	#endif
#endif

// Switch (no read, read, read chunked, can read)

// These are exclusive states
namespace Mode {
	enum e_http_mode {
		GET = 1 << 0,
		POST = 1 << 1,
		DELETE = 1 << 2,
		CGI = 1 << 3,
		SSE = 1 << 4
	};
}

namespace State {
	enum e_http_state {
		READING_FROM_CLIENT = 1,
		WRITING_TO_CLIENT = 2,
		PARSING = 4,
		FIRST_LINE = 8,
		CLOSE = 16
	};
}

namespace Options {
	enum e_http_options {
		CHUNKED_LENGTH = 1 << 0,
		FIXED_LENGTH = 1 << 1,
		HOST = 1 << 2,
		CONNECTION_TYPE = 1 << 3	// Keep alive or
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
		COOKIES,
		COUNT
	};
}

#define MIME_TABLE {"html", "htm", "css", "json", \
	"js", "png", "jpg", "jpeg", "gif", "txt"};

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

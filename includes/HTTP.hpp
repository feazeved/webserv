#pragma once

#include <string>
#include <vector>
#include <map>
#include <unistd.h>
#include "core.hpp"
#include "Status.hpp"

namespace HTTP {

struct Location {
	std::string					path;
	std::string					root;
	std::string					index;
	std::string					upload_store;
	u8							methods;
	bool						autoindex;
	Status						redirect;

	Location() : autoindex(false) {}
};

struct ServerConfig {
	std::map<long, std::string>	errors;
	std::vector<Location>		locations;
	std::string					host;
	long						port;
	long						maxBodySize;
	bool						autoindex;

	ServerConfig() : host("localhost"), port(-1), maxBodySize(-1), autoindex(false) {}
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
// 

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

namespace Field {
	enum e_http_field {
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

namespace Mime {
	enum e_mime_type {
		HTML = 0,
		HTM,
		CSS,
		JSON,
		JS,
		PNG,
		JPG,
		JPEG,
		GIF,
		TXT,
		OCTET_STREAM,
		COUNT
	};
}
}

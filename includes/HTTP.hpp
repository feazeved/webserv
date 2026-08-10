#pragma once

#include <string>
#include <vector>
#include <map>

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

namespace Mode {
	enum e_http_mode {
		GET = 1 << 0,
		POST = 1 << 1,
		DELETE = 1 << 2,
		CGI = 1 << 3,
		SSE = 1 << 4
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

namespace Options {
	enum e_http_options {
		CHUNKED_LENGTH = 1 << 0,
		FIXED_LENGTH = 1 << 1,
		HOST = 1 << 2,
		CONNECTION_TYPE = 1 << 3
	};
}

namespace State {
	enum e_http_state {
		ERROR = 0,
		READING_FROM_CLIENT = 1,
		WRITING_TO_CLIENT = 2,
		FIRST_LINE = 4,
		PARSING = 8,
		DONE = 16
	};
}

}

#pragma once

#include <string>
#include <vector>
#include <map>

namespace HTTP {

struct Location {
	std::vector<std::string>	methods;
	std::string					path;
	std::string					root;
	std::string					index;
	std::string					upload_store;
	bool						autoindex;

	Location() : autoindex(false) {}
};

struct ServerConfig {
	std::map<long, std::string>	errors;
	std::vector<Location>		locations;
	std::string					host;
	long						port;
	long						maxBodySize;

	ServerConfig() : host("localhost"), port(-1), maxBodySize(-1) {}
};

#define STATUS_200 "200 OK\r\n"
#define STATUS_201 "201 Created\r\n"
#define STATUS_204 "204 No Content\r\n"
#define STATUS_301 "301 Moved Permanently\r\n"
#define STATUS_302 "302 Found\r\n"
#define STATUS_400 "400 Bad Request\r\n"
#define STATUS_403 "403 Forbidden\r\n"
#define STATUS_404 "404 Not Found\r\n"
#define STATUS_405 "405 Method Not Allowed\r\n"
#define STATUS_411 "411 Length Required\r\n"
#define STATUS_413 "413 Content Too Large\r\n"
#define STATUS_414 "414 URI Too Long\r\n"
#define STATUS_431 "431 Request Header Fields Too Large\r\n"
#define STATUS_500 "500 Internal Server Error\r\n"
#define STATUS_501 "501 Not Implemented\r\n"
#define STATUS_502 "502 Bad Gateway\r\n"
#define STATUS_504 "504 Gateway Timeout\r\n"
#define STATUS(code) HTTP_STATUS_##code

namespace Attributes {

enum Attributes {
	METHOD_GET = 1 << 0,
	METHOD_POST = 1 << 1,
	METHOD_DELETE = 1 << 2,
	CGI = 1 << 3,
	HOST = 1 << 4,
	CHUNKED = 1 << 5,
	FOO = 1 << 6
};

enum State {
	READING = 1 << 0,		// Reading header
	PROCESSING = 1 << 1,	// Reading body
	WRITING = 1 << 2,		// Writing

	PROCESSING_LENGTH = 1 << 6, // Reading the header for the body
	FIRST_LINE = 1 << 7
};
}
}

#pragma once

#include <string>
#include <vector>
#include <map>

#include "core.hpp"
#include "Status.hpp"

namespace HTTP {

// It needs to be like the one in Connection.hpp...
// Better have only one but this works
enum Method {
	GET = 1 << 0,
	POST = 1 << 1,
	DELETE = 1 << 2,
};

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

#define STATUS_200 "200 OK"
#define STATUS_201 "201 Created"
#define STATUS_204 "204 No Content"
#define STATUS_301 "301 Moved Permanently"
#define STATUS_302 "302 Found"
#define STATUS_400 "400 Bad Request"
#define STATUS_403 "403 Forbidden"
#define STATUS_404 "404 Not Found"
#define STATUS_405 "405 Method Not Allowed"
#define STATUS_411 "411 Length Required"
#define STATUS_413 "413 Content Too Large"
#define STATUS_414 "414 URI Too Long"
#define STATUS_431 "431 Request Header Fields Too Large"
#define STATUS_500 "500 Internal Server Error"
#define STATUS_501 "501 Not Implemented"
#define STATUS_502 "502 Bad Gateway"
#define STATUS_504 "504 Gateway Timeout"
#define STATUS(code) HTTP_STATUS_##code

}

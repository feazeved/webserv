#pragma once

#include <string>
#include <vector>
#include <map>

#include "core.hpp"

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
	bool						autoindex;

	ServerConfig() : host("localhost"), port(-1), maxBodySize(-1), autoindex(false) {}
};

enum Direction {
	LEFT,
	RIGHT,
	UP,
	DOWN
};

struct Player {
	i32			id;
	std::string	nick;
	f64			x;
	f64			y;
	Direction	direction;
	bool		moving;

	Player() : x(0), y(0), direction(DOWN), moving(false) {}
};

enum GameEvent {
	JOIN,
	QUIT,
	MOVE
};

struct GameEvent {
	EventType	type;
	Player*		player;
};

namespace Attributes {

enum Attributes {
	METHOD_GET = 1 << 0,
	METHOD_POST = 1 << 1,
	METHOD_DELETE = 1 << 2,
	CGI = 1 << 3,
	VERSION = 1 << 4,
	HOST = 1 << 5,
	CHUNKED = 1 << 6,
	FOO = 1 << 7
};

enum State {
	READING = 1 << 0,		// Reading header
	PROCESSING = 1 << 1,	// Reading body
	WRITING = 1 << 2,		// Writing
	FIRST_LINE = 1 << 7
};
}
}

#pragma once

#include <unistd.h>
#include <sys/epoll.h>

#include "HTTP.hpp"
#include "core.hpp"
#include "Connection_helpers.ipp"
#include "Buffer.hpp"
#include <ctime>

namespace Game { class State; }

namespace HTTP {

namespace Attributes {
	enum Attributes {
		GET = 1 << 0,
		POST = 1 << 1,
		DELETE = 1 << 2,
		CGI = 1 << 3,
		HOST = 1 << 4,
		CHUNKED = 1 << 5,
		DONE = 1 << 7
	};
}

typedef struct {
	struct {
		u32 index;
		u32 size;
	} path, query, cookie;
} RequestVars;

typedef struct {
	usize bodySizeMax;
} t_servcfg;

template <usize bufferSize>
class Connection {
public:
	RequestVars vars;
	usize bodySize, chunkSize;
	ServerConfig* cfg;
	Buffer<bufferSize> clientInput, clientOutput;

	time_t startTime, cgiStartTime;
	time_t bonusTime;	// Value ranging from -30s to 30s
	pid_t processId;

	struct {
		i32 client;		// Duplex FD
		i32 writeEnd;	// CGI Input or POST
		i32 readEnd;	// CGI Output or GET/DEL
	} fd;

	union {
		u64 state;
		struct {
			u32 metadata;
			u16 status;
			u8 info;
			u8 type;
		};
	};

public:
	Game::State* gameState;
	bool isSSE;
	bool headerParsed;
	std::string sse_buffer;

	i32 dispatch() {

	}

	// TODO
	i32 init(i32 f, ServerConfig* c) {
		(void)f;
		cfg = c;
		return 1;
	}

	// TODO
	i32 clear() {
		return 1;
	}


	// Game
	i32 handle_game_request();

	// Parsing
	bool  checkType(const std::string& method, std::vector<std::string>::iterator& mit, std::vector<std::string>::iterator& end);
	bool  checkLocation();
	isize parse_header(usize bytes, u32 events);
	isize parse_first_line(char *str, char *end);
	isize parse_line(char *str, char *end);
	isize parse_target(char *str, char *end);

	// Configuration
	isize error_path();
	isize configure();
	void  buildHeader();
	isize buildCgiHeader();
	isize cgi_first_run();
	isize get_first_run();
	isize post_first_run();
	isize del_first_run();

	// HTTP Methods
	isize del_method(usize bytes, u32 events);
	isize get_method(usize bytes, u32 events);
	isize post_method(usize bytes, u32 events);
	isize cgi_method(usize bytes, u32 events);

	// Common
	isize read_from_server(usize bytes);
	isize write_to_server(usize bytes);
	isize write_to_client(usize bytes, u32 events);
	isize read_from_client(usize bytes, u32 events);
	isize dechunk(usize bytes, Buffer<bufferSize>& src);

	// ======== Constructors ====================
	Connection() :
		bodySize(SIZE_MAX),
		type(0),
		gameState(NULL),
		isSSE(false),
		headerParsed(false) {
	}
};

} // namespace HTTP

#include "Connection_parse.ipp"
#include "Connection_configure.ipp"
#include "Connection_methods.ipp"
#include "Connection_common.ipp"
#include "Connection_cgi.ipp"
#include "Connection_game.ipp"

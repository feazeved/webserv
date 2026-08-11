#pragma once

#include <unistd.h>
#include <sys/epoll.h>
#include <ctime>
#include "HTTP.hpp"
#include <fcntl.h>
#include <vector>
#include <cstring>
#include <sys/stat.h>
#include <string>

#include "HTTP.hpp"
#include "core.hpp"
#include "Buffer.hpp"
#include "Request.hpp"
#include "State.hpp"

#define CONNECTION_INL(ret_type) ret_type inline HTTP::Connection::

namespace HTTP {

class Connection {
public:
	Buffer<16384> clientInput, clientOutput;

	ServerConfig* cfg;
	Game::State* gameState;

	Request request;
	u8 state;

	time_t startTime, cgiStartTime, bonusTime; // Value ranging from -30s to 30s

	pid_t processId;
	i32 clientFd;	// Duplex FD
	i32 writeFd;	// CGI Input or POST
	i32 readFd;		// CGI Output or GET/DEL

	bool isSSE;
	std::string sse_buffer;	// TODO: find out what this is

	isize dispatch(u32 events);

	// TODO
	isize init(i32 f, ServerConfig* c) {
		(void)f;
		cfg = c;
		return 1;
	}

	// TODO
	isize clear();

	// Game
	i32 handle_game_request();

	// Configuration
	isize configure();
	isize error_path();
	void  build_header(usize contentLength = SIZE_MAX, u8 mimeIndex = Mime::OCTET_STREAM);
	isize build_cgi_header();

	// HTTP Methods
	isize del_method();
	isize del_first_run();
	isize get_method();
	isize get_autoindex();
	isize get_first_run();
	isize post_method();
	isize post_first_run();
	isize cgi_method();
	isize cgi_first_run();
	isize sse_method();

	// Common
	isize read_from_server();
	isize write_to_server();
	isize write_to_client(u32 events);
	isize read_from_client(u32 events);

	isize dechunk(Cursor& src, Cursor& dst);
	isize decode();

	// ======== Constructors ====================
	// Connection() :
	// 	gameState(NULL),
	// 	isSSE(false),
	// 	headerParsed(false) {
	// }
};

// namespace HTTP
}

#include "Connection_fs_helpers.ipp"
#include "Connection_configure.ipp"
#include "Connection_common.ipp"
#include "Connection_game.ipp"

#include "Connection_cgi.ipp"
#include "Connection_get.ipp"
#include "Connection_post.ipp"
#include "Connection_delete.ipp"

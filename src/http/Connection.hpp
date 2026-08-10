#pragma once

#include <unistd.h>
#include <sys/epoll.h>
#include <ctime>

#include "HTTP.hpp"
#include "core.hpp"
#include "Buffer.hpp"
#include "Request.hpp"

#define CONNECTION_INL(ret_type) template <usize bufferSize> ret_type inline HTTP::Connection<bufferSize>::

namespace Game { class State; }	// alex: ta certo isso? wtf

namespace HTTP {

template <usize bufferSize>
class Connection {
public:
	ServerConfig* cfg;
	Game::State* gameState;

	Buffer<bufferSize> clientInput, clientOutput;
	Request<bufferSize> request;

	time_t startTime, cgiStartTime, bonusTime; // Value ranging from -30s to 30s

	pid_t processId;
	i32 clientFd;	// Duplex FD
	i32 writeFd;	// CGI Input or POST
	i32 readFd;		// CGI Output or GET/DEL

	bool isSSE;
	std::string sse_buffer;	// TODO: find out what this is

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

	// Configuration
	isize error_path();
	isize configure();
	void  build_header();
	isize get_first_run();
	isize post_first_run();
	isize del_first_run();

	// HTTP Methods
	isize del_method(usize bytes, u32 events);
	isize get_method(usize bytes, u32 events);
	isize post_method(usize bytes, u32 events);

	// CGI
	isize cgi_first_run();
	isize cgi_method(usize bytes, u32 events);
	isize build_cgi_header();

	// Common
	isize read_from_server(usize bytes);
	isize write_to_server(usize bytes);
	isize write_to_client(usize bytes, u32 events);
	isize read_from_client(usize bytes, u32 events);
	isize dechunk(usize bytes, Buffer<bufferSize>& src);

	// ======== Constructors ====================
	// Connection() :
	// 	gameState(NULL),
	// 	isSSE(false),
	// 	headerParsed(false) {
	// }
};

} // namespace HTTP

#include "Connection_configure.ipp"
#include "Connection_methods.ipp"
#include "Connection_common.ipp"
#include "Connection_cgi.ipp"
#include "Connection_game.ipp"

#pragma once
#include "core.hpp"
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
	static const usize metadataSize = sizeof(ServerConfig*) + sizeof(Request) + sizeof(time_t) +
		2 * sizeof(u8) + sizeof(pid_t) + 3 * sizeof(i32) + sizeof(std::string) + sizeof(bool);
	static const usize metasizeAlign = ALIGN_UP(metadataSize / 2, 8ul);
	static const usize bytesFree = 2 * metasizeAlign - metadataSize;	// Debug only
	static const usize bufferSize = HTTP_BUFFERSIZE - metasizeAlign;

public:
	Buffer<bufferSize> clientInput, clientOutput;

	ServerConfig* cfg;
	Request request;
	time_t startTime;
	u8 state;
	u8 bonusTime; // Value ranging from -30s to 30s

	pid_t processId;
	i32 clientFd;	// Duplex FD
	i32 writeFd;	// CGI Input or POST
	i32 readFd;		// CGI Output or GET/DEL

	bool isSSE;				// TODO: these vars should be removed, is sse should belong in request.mode (it already has the enum)
	std::string sse_buffer;	// This one too

	isize dispatch(u32 events);

	// TODO
	isize init(i32 f, ServerConfig* c) {
		(void)f;
		cfg = c;
		return 1;
	}

	void reset() {
		if (readFd >= 0)
			close(readFd);
		if (writeFd >= 0)
			close(writeFd);
		writeFd = -1;
		readFd = -1;
		clientFd = -1;	// If its not closed here we're in big trouble
		processId = -1;
		bonusTime = 0;
		startTime = 0;
		cfg = NULL;

		request.reset();
	}

	// TODO
	isize clear();

	// Game
	i32 handle_game_request();

	// Configuration
	isize configure();
	isize error_path();
	void  build_header();	// Alex: Isso tem que tar no init do request, mime_index = contentType, contentLength = bodySize
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

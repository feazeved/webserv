#pragma once
#include "core.hpp"
#include <unistd.h>
#include <sys/epoll.h>
#include <ctime>
#include "HTTP.hpp"
#include <fcntl.h>
#include <cstring>
#include <sys/stat.h>
#include <string>

#include "HTTP.hpp"
#include "core.hpp"
#include "Buffer.hpp"
#include "VirtualServer.hpp"

#include "Request.hpp"
#include "State.hpp"

#define CONNECTION_INL(ret_type) ret_type inline HTTP::Connection::

// For epoll conformity, cgi should be an object of Connection, 
// and it gets added and removed to epoll
/*
	Transaction Context: bonusTime, time, cfg, state

	Execution Context: clientFd, writeFd, readFd, processId
*/

namespace HTTP {

class Connection {
public:
	static const usize metadataSize = sizeof(VirtualServer*) + sizeof(Request) 
		+ sizeof(time_t) + 2 * sizeof(u8) + sizeof(pid_t) + 3 * sizeof(i32);
	static const usize metasizeAlign = ALIGN_UP(metadataSize / 2, 8ul);
	static const usize bytesFree = 2 * metasizeAlign - metadataSize;	// Debug only
	static const usize bufferSize = HTTP_BUFFERSIZE - metasizeAlign;

	typedef Buffer<bufferSize> HTTP_Buffer;
	// Actually the buffer should be here like
	// u8 rawData[32768];

public:
	VirtualServer* cfg;
	HTTP_Buffer recvBuffer, sendBuffer;
	Request request;

	time_t startTime;
	u8 bonusTime; // Value ranging from -30s to 30s
	u8 state;

	pid_t processId;

	// TODO: These FDs can be moved to the buffer
	i32 clientFd;	// Duplex FD
	i32 writeFd;	// CGI Input or POST
	i32 readFd;		// CGI Output or GET/DEL

	isize dispatch(u32 events);

	// TODO
	isize init(i32 f, VirtualServer* c) {
		(void)f;
		cfg = c;
		return 1;
	}

	bool check_timeout(time_t curTime, time_t maxTime) {
		const time_t elapsed = curTime - startTime;

		if (elapsed > maxTime + bonusTime) {
			// TODO: Cull child here
			return true;
		}
		return false;
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
	void  build_header();
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

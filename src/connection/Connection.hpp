#pragma once
#include "core.hpp"
#include "Buffer.hpp"
#include "VirtualServer.hpp"
#include "Request.hpp"
#include "Connection_helpers.ipp"

#define CONNECTION_INL(ret_type) ret_type inline Connection::

class Connection {
public:
	static const usize metadataSize = sizeof(VirtualServer*) + sizeof(Request) 
		+ sizeof(time_t) + 2 * sizeof(u8) + sizeof(pid_t) + 3 * sizeof(int);
	static const usize metasizeAlign = ALIGN_UP(metadataSize / 2, 8ul);
	static const usize bytesFree = 2 * metasizeAlign - metadataSize;	// Debug only
	static const usize bufferSize = HTTP_BUFFERSIZE - metasizeAlign;

public:
	VirtualServer* cfg;
	HTTP_Buffer recvBuffer, sendBuffer;
	Request request;

	time_t startTime;
	u8 bonusTime; // Value ranging from -30s to 30s
	Mode::e_http_mode mode;

	pid_t processId;
	int clientFd;	// Duplex FD
	int writeFd;	// CGI Input or POST
	int readFd;		// CGI Output or GET/DEL

	isize dispatch(u32 events);

	isize init(int fd, VirtualServer* c) {
		ASSERT(clientFd != -1, "Assigned a connection already in use");
		request.reset();
		clientFd = fd;
		cfg = c;
		readFd = -1;
		writeFd = -1;
		processId = -1;
		mode = Mode::PARSE;
		recvBuffer.clear();
		sendBuffer.clear();
		startTime = 0;
		bonusTime = 0;
		return 1;
	}

	void clear() {
		if (readFd >= 0)
			close(readFd);
		if (writeFd >= 0)
			close(writeFd);
		clientFd = -1;
	}

	bool check_timeout(time_t curTime) {
		const time_t elapsed = curTime - startTime;

		if (elapsed > CONNECTION_TIMEOUT + bonusTime) {
			// TODO: Cull child here
			return true;
		}
		return false;
	}

	// Game
	int handle_game_request();

	// Configuration
	isize parse(u32 events);
	isize error_path();
	void build_header();
	isize build_cgi_header();
	void build_path(char* buffer, const char* ptr, usize length);

	// HTTP Methods
	isize upload_file(u32 events);
	isize download_file(u32 events);

	isize cgi_method();
	isize sse_method();
	
	isize get_directory(struct stat *st);
	isize del_first_run();
	isize get_first_run();
	isize post_first_run();
	isize cgi_first_run();
	isize get_autoindex();

	isize write_to_client(u32 events);
	isize read_from_client(u32 events);
	isize close_connection();

	Connection() : clientFd(-1) {}
};

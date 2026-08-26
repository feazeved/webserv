#pragma once
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>

#include "core.hpp"
#include "Clock.hpp"
#include "Buffer.hpp"
#include "VirtualServer.hpp"
#include "Request.hpp"
#include "Environment.hpp"

#define CONNECTION_INL(ret_type) ret_type inline Connection::

class Connection {
public:
	static const usize metadataSize = sizeof(VirtualServer*) + sizeof(Request) 
		+ sizeof(time_t) + 2 * sizeof(u8) + sizeof(pid_t) + 3 * sizeof(int);
	static const usize metasizeAlign = ALIGN_UP(metadataSize / 2, 8ul);
	static const usize bytesFree = 2 * metasizeAlign - metadataSize;	// Debug only
	static const usize bufferSize = HTTP_BUFFERSIZE - metasizeAlign;

public:
	static Environment s_fakeEnv;
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
		startTime = Clock::time_elapsed();
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

	void append_env(Buffer64 &buffer, char* argv[3]);
	pid_t exec_script(char *const argv[3], int fdIn[2], int fdOut[2]);
	
	// Configuration
	isize dispatch(u32 events);
	isize parse(u32 events);
	isize error_path();
	void build_header();
	isize build_cgi_header();
	isize close_connection();

	// Common
	isize write_to_client(u32 events);
	isize read_from_client(u32 events);

	// Streaming
	isize upload_file(u32 events);
	isize download_file(u32 events);
	isize cgi_method();
	isize sse_method();

	// First Run
	isize del_first_run();
	isize get_first_run();
	isize post_first_run();
	isize cgi_first_run();
	isize get_autoindex(struct stat &st, Buffer16 &pathBuffer);
	isize get_directory(struct stat &st, Buffer16 &pathBuffer);

	Connection() : clientFd(-1) {}
};

#include "Connection_common.ipp"
#include "Connection_dispatch.ipp"
#include "Connection_methods.ipp"
#include "Connection_response.ipp"
#include "Connection_autoindex.ipp"
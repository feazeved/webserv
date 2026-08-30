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
#include "Environment.hpp"
#include "Epoll.hpp"

#define CONNECTION_INL(ret_type) inline ret_type Connection::

class Connection {
public:
	struct Request {
		Span path, query, cookies, interpreter;
		Span contentTypeHeader, contentSize;
		Location* location;

		void reset() {
			MEMSET_INLINE(this, 0, sizeof(*this));
		}
	};

	static const usize metadataSize = 160;	// TODO: adjust size on final pass
	static const usize metasizeAlign = ALIGN_UP(metadataSize / 2, 8ul);
	static const usize bytesFree = 2 * metasizeAlign - metadataSize;	// Debug only
	static const usize bufferSize = HTTP_BUFFERSIZE - metasizeAlign;

public:
	HTTP_Buffer recvBuffer;
	union {
		HTTP_Buffer sendBuffer;	// Request shares memory with sendBuffer
		Request req;	// Req values are not needed during execution
	};

	VirtualServer* cfg;
	Status status;
	u8 options;
	u8 contentType;
	Mode::e_http_mode mode;
	i32 clientFd;
	u32 startTime;

	pid_t processId;
	usize bodySize;

	union {
		struct {
			usize chunkSize; 
			i32 readFd, writeFd;
		};
		Span32 uri;	// TODO: Store the uri on setup
		DIR* directory;
	};

	isize init(int fd, VirtualServer* serverConfig) {
		ASSERT(clientFd != -1, "Assigned a connection already in use");
		clientFd = fd;
		cfg = serverConfig;
		readFd = -1;
		writeFd = -1;
		processId = -1;
		mode = Mode::PARSE;
		recvBuffer.clear();
		sendBuffer.clear();
		startTime = Clock::time_elapsed();
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

		if (elapsed > CONNECTION_TIMEOUT) {
			// TODO: Cull child here
			return true;
		}
		return false;
	}

	void append_env(Buffer64 &buffer, char* argv[3]);
	Location* check_location();

	// Parsing
	isize validate_target();
	isize parse_first_line(usize lineLength);
	isize parse_line(usize lineLength);
	isize parse_cgi_line(Buffer16 &tmpBuffer);
	isize validate_header();

	// Configuration
	isize dispatch(Epoll &epoll);
	isize parse(Epoll &epoll);
	isize error_path();
	void build_header();
	isize build_cgi_header();
	isize close_connection();

	// Common
	isize write_to_client(Epoll &epoll);
	isize read_from_client(Epoll &epoll);

	// Streaming
	isize upload_file(Epoll &epoll);
	isize download_file(Epoll &epoll);
	isize cgi_method();
	isize sse_method();

	// First Run (rename to setup)
	isize del_setup();
	isize get_setup();
	isize post_setup();
	isize cgi_setup();
	
	isize get_autoindex();
	isize get_directory(struct stat &st, Buffer8 &pathBuffer);
};

#include "Connection_common.ipp"
#include "Connection_dispatch.ipp"
#include "Connection_methods.ipp"
#include "Connection_response.ipp"
#include "Connection_setup.ipp"
#include "Connection_autoindex.ipp"
#include "Connection_validate.ipp"
#include "Connection_parse.ipp"
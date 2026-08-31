#pragma once
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
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

static const usize metadataSize = 160;	// TODO: adjust size on final pass
static const usize metasizeAlign = ALIGN_UP(metadataSize / 2, 8ul);
static const usize bytesFree = 2 * metasizeAlign - metadataSize;	// Debug only
static const usize bufferSize = HTTP_BUFFERSIZE - metasizeAlign;
typedef Buffer<bufferSize> HTTP_Buffer;

struct Connection {
	struct Request {
		Span target, query, cookies, interpreter;
		Span contentTypeHeader, contentSize;
		Location* location;

		// Can add root and other location vars here

		void reset() {
			MEMSET_INLINE(this, 0, sizeof(*this));
		}
	};

	VirtualServer* cfg;
	Status status;
	u16 options;	// TODO: Change this to a bitmap
	u8 contentType;	// TODO: this should be a span in req maybe?
	Mode::e_http_mode mode;
	u32 startTime;
	pid_t processId;
	usize bodySize;
	i32 clientFd, readFd, writeFd;
	HTTP_Buffer recvBuffer;

	union {
		HTTP_Buffer sendBuffer;		// Request shares memory with sendBuffer
		struct {
			u8 padding[bufferSize - sizeof(Request)];
			Request req;			// Req values are not needed during execution
		};
	};
	union {
		usize chunkSize;
		DIR* directory;
	};

	// Common
	isize init(int fd, VirtualServer* serverConfig);
	void clear();
	bool check_timeout(time_t curTime);

	// Parsing
	isize parse_first_line(usize lineLength);
	isize parse_line(usize lineLength);
	isize parse_cgi_line(Buffer64 &tmpBuffer);

	// Validation
	isize validate_target(usize lineEnd);
	isize validate_header(Epoll &epoll);
	Location* check_location();

	// Configuration
	isize dispatch(Epoll &epoll);
	isize parse(Epoll &epoll);
	isize close_connection(bool streamHeader=true);

	// Response
	void build_error_header();
	void build_header();
	isize build_cgi_header();

	// Common
	isize write_to_client(Epoll &epoll);
	isize read_from_client(Epoll &epoll);

	// Streaming
	isize upload_file(Epoll &epoll);
	isize download_file(Epoll &epoll);
	isize cgi_method(Epoll &epoll);
	isize upload_directory(Epoll &epoll);

	// Setup
	isize setup(Epoll &epoll);
	isize del_setup(Epoll &epoll);
	isize get_setup(Epoll &epoll);
	isize post_setup(Epoll &epoll);
	isize cgi_setup(Epoll &epoll);
	char* append_env(Buffer64 &buffer, char* argv[3]);
	isize get_directory(Epoll &epoll, struct stat &st, Buffer64 &pathBuffer);
};

#include "Connection_common.ipp"
#include "Connection_dispatch.ipp"
#include "Connection_stream.ipp"
#include "Connection_stream_get.ipp"
#include "Connection_response.ipp"
#include "Connection_setup.ipp"
#include "Connection_setup_cgi.ipp"
#include "Connection_parse.ipp"
#include "Connection_validate.ipp"

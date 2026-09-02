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

#include "match.hpp"

#define CONNECTION_INL(ret_type) inline ret_type Connection::

static const usize metadataSize = 160;	// TODO: adjust size on final pass
static const usize metasizeAlign = ALIGN_UP(metadataSize / 2, 8ul);
static const usize bytesFree = 2 * metasizeAlign - metadataSize;	// Debug only
static const usize bufferSize = HTTP_BUFFERSIZE - metasizeAlign;
typedef Buffer<bufferSize> HTTP_Buffer;

struct Connection {
	struct Request {
		Span target, query, host, cookies, interpreter;
		Span contentTypeHeader, contentSize;	// relativeTarget (what comes after URI)
		Location* location;
		Span relativeTarget, targetName, targetExt;
		Span uri, cgi;

		// Can add root and other location vars here

		void clear() {
			MEMSET_INLINE(this, 0, sizeof(*this));
		}
	};

	VirtualServer* cfg;
	Status status;
	u16 options;	// TODO: Change this to a bitmap
	u8 contentType;	// TODO: this should be a span in req maybe?
	Mode::e_http_mode mode;
	u32 startTime;
	u8 ioState;
	usize bodySize;
	i32 clientFd, readFd;
	HTTP_Buffer recvBuffer;

	union {
		dirent* dirEntry;
		struct { pid_t processId; i32 writeFd; };
	};

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

	// Parsing
	isize parse_line(usize lineLength);
	isize parse_first_line(usize lineLength);
	isize parse_cgi_line(Buffer64 &tmpBuffer);
	isize validate_target(char* str, char* end);
	bool match_location();
	Span check_cgi();

	// Configuration
	isize dispatch(Epoll &epoll);
	isize parse(Epoll &epoll);
	isize end_connection();

	// Response
	void build_error_header();
	void build_header(Status::Code code);
	isize build_cgi_header();

	// Common
	isize flush(Epoll &epoll);
	isize write_to_client(Epoll &epoll);
	isize read_from_client(Epoll &epoll);
	char* append_target_path(Buffer64 &buffer);

	// Streaming
	isize cgi(Epoll &epoll);
	isize upload_file(Epoll &epoll);
	isize download_file(Epoll &epoll);
	isize upload_directory(Epoll &epoll);

	// Setup
	isize setup(Epoll &epoll);
	isize del_setup(Epoll &epoll);
	isize get_setup(Epoll &epoll);
	isize get_directory_setup(Epoll &epoll, struct stat &st, Buffer64 &pathBuffer);
	isize post_setup(Epoll &epoll);
	isize cgi_setup(Epoll &epoll);
	char* append_env(Buffer64 &buffer, char* argv[3]);
	isize flush_setup(Epoll &epoll, Status::Code code);
	isize flush_setup_close(Epoll &epoll, Status::Code code);
	isize parse_setup(Epoll &epoll);
};

#include "Connection_common.ipp"
#include "Connection_dispatch.ipp"
#include "Connection_response.ipp"
#include "Connection_stream.ipp"
#include "Connection_setup.ipp"
#include "Connection_parse.ipp"
#include "Connection_parse_first.ipp"

#include "Connection_mode_get.ipp"
#include "Connection_mode_autoindex.ipp"
#include "Connection_mode_post.ipp"
#include "Connection_mode_cgi.ipp"

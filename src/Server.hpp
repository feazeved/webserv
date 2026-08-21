#pragma once

#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "HTTP.hpp"
#include "State.hpp"
#include "Connection.hpp"
#include "BlockVector.hpp"
#include "core.hpp"
#include "Server_helpers.ipp"

#include "VirtualServer.hpp"

#define SERVER_INL(ret_type) ret_type inline HTTP::Server::

namespace HTTP {

class Server {
public:
	static const usize s_connectionBlockSize = 32;
	static const usize s_connectionMaxGrowth = 64;
	static const usize s_maxEvents = 16;
	typedef BlockVector<HTTP::Connection, s_connectionBlockSize, s_connectionMaxGrowth> connectionPool;

	struct Token {
		enum Type {
			OPEN_BRACKET,
			CLOSE_BRACKET,
			SEMICOLON,
			WORD
		}	type;
		StringView value;
	};

	struct Directive {
		StringView name;
		Array32<StringView> args;
	};

public:
	usize fileOffset;
	usize fileSize;
	usize serverCount;
	VirtualServer servers[MAX_VIRTUAL_SERVERS];
	connectionPool connections;
	i32 epollFd;

	char* getPtr() {
		return fileOffset + (char*) Arena::data;
	}

	Server(const char *filePath)
		: fileOffset(0), fileSize(0), serverCount(0), epollFd(-1) {
		read_whole_file(filePath);
		serverCount = s_count_servers(getPtr(), fileSize);
		if (serverCount == 0 || serverCount > MAX_VIRTUAL_SERVERS)
			PERR_EXIT(cleanup(), "Error: Invalid config");

		parse_config(servers);
		epollFd = epoll_create(1);
		if (epollFd == -1)
			PERR_EXIT(cleanup(), "Error: Failed to create epoll instance");

		for (usize index = 0; index < serverCount; index++)
			servers[index].init();
	}

	~Server() {
		cleanup();
	}

	int cleanup() {
		if (epollFd != -1) {
			close(epollFd);
			epollFd = -1;
		}
		for (usize index = 0; index < connections.capacity(); index++) {
			if (connections.metadata.bitread(index))
				connections.clear(index);	// TODO: move this to block vector
		}
		for (usize index = 0; index < serverCount; index++)
			servers[index].cleanup();
		serverCount = 0;
		Arena::clear();
		return 1;
	}

	// Execution
	void run();
	void mark_connection_writable(usize connectionIndex);
	void add_to_epoll(i32 fd, u32 events, u64 key);
	void remove_from_epoll(i32 fd);
	void modify_epoll_event(usize connectionIndex, u32 events);
	void dispatch_epoll_event(const struct epoll_event& event);
	void dispatch_connection_event(usize index, u32 events);
	void add_connection(VirtualServer* server);
	void close_connection(usize connectionIndex);

	// Parsing
	void read_whole_file(const char* filePath);
	usize get_next_word(char* &ostr);
	Token match_delimiter(char *ptr, usize delimPos, isize &braces);
	Array32<Token> tokenize();
	usize find_scope_end(const Array32<Token> &tokens, usize begin, usize end);
	usize count_locations(const Array32<Token> &tokens, usize cursor, usize end);
	void set_methods(Array32<StringView> &methods, HTTP::Location &location);
	isize set_location_directive(Directive &dir, HTTP::Location &location);
	isize set_server_directive(Directive &dir, HTTP::ServerConfig &server);
	isize parse_directive(const Array32<Token> &tokens, usize &cursor, usize end, Directive &dir);
	isize parse_location(const Array32<Token> &tokens, usize &cursor, usize end, HTTP::Location &loc);
	isize parse_server(const Array32<Token> &tokens, usize cursor, usize end, HTTP::ServerConfig &server);
	void parse_config(VirtualServer (&servers)[MAX_VIRTUAL_SERVERS]);
};

}

#include "Server_init.ipp"
#include "Server_epoll.ipp"
#include "Server_dispatch.ipp"
#include "Server_parse.ipp"

#pragma once

#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h>
#include "HTTP.hpp"
#include "State.hpp"
#include "Connection.hpp"
#include "core.hpp"
#include "Server_helpers.ipp"
#include "ConnectionPool.hpp"
#include "VirtualServer.hpp"
#include "Parser.hpp"

#define SERVER_INL(ret_type) ret_type inline HTTP::Server::

namespace HTTP {

class Server {
public:
	static const usize s_maxEvents = 16;

public:
	Parser parser;
	VirtualServer servers[MAX_VIRTUAL_SERVERS];
	ConnectionPool connections;
	i32 epollFd;

	Server(const char *filePath)
		: parser(filePath, servers), epollFd(-1) {

		epollFd = epoll_create(1);
		if (epollFd == -1)
			PERR_EXIT(clear(), "Error: Failed to create epoll instance");

		for (usize index = 0; index < parser.serverCount; index++)
			servers[index].init();
	}

	~Server() {
		clear();
	}

	int clear() {
		if (epollFd != -1) {
			close(epollFd);
			epollFd = -1;
		}
		connections.clear();
		for (usize index = 0; index < parser.serverCount; index++)
			servers[index].clear();
		parser.serverCount = 0;
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
};

}

#include "Server_epoll.ipp"
#include "Server_dispatch.ipp"

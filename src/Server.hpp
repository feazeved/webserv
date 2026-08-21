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

#include "Parser.hpp"
#include "VirtualServer.hpp"

#define SERVER_INL(ret_type) ret_type inline HTTP::Server::

namespace HTTP {

class Server {
public:
	static const usize s_connectionBlockSize = 32;
	static const usize s_connectionMaxGrowth = 64;
	static const usize s_maxEvents = 16;
	typedef BlockVector<HTTP::Connection, s_connectionBlockSize, s_connectionMaxGrowth> connectionPool;

public:
	Parser file;
	VirtualServer servers[MAX_VIRTUAL_SERVERS];
	usize serverCount;
	connectionPool connections;
	i32 epollFd;

	Server(const char *filePath)
		: file(filePath, serverCount), serverCount(0), connections(), epollFd(-1) {

		file.parse_config(servers);
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
		for (usize index = 0; index < serverCount; index++)
			servers[index].cleanup();
		serverCount = 0;
		return 1;
	}

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

#include "Server_epoll.ipp"
#include "Server_dispatch.ipp"
}
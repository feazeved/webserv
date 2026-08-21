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

#define SERVER_INL(ret_type) ret_type inline Server::

class Server {
public:
	static const usize s_maxEvents = 16;
	static const usize s_connectionBlockSize = 32;
	static const usize s_connectionMaxGrowth = 64;

	VirtualServer servers[MAX_VIRTUAL_SERVERS];
	BlockVector<HTTP::Connection, s_connectionBlockSize, s_connectionMaxGrowth> connections;
	usize serverCount;
	i32 epollFd;

	Server(HTTP::ServerConfig (&configs)[MAX_VIRTUAL_SERVERS], usize numServers)
		: connections(), serverCount(0), epollFd(-1) {
		if (numServers == 0 || numServers > MAX_VIRTUAL_SERVERS)
			PERR_EXIT(cleanup(), "Error: Invalid virtual server count");

		epollFd = epoll_create(1);
		if (epollFd == -1)
			PERR_EXIT(cleanup(), "Error: Failed to create epoll instance");

		serverCount = numServers;
		for (usize index = 0; index < serverCount; index++)
			servers[index].init(configs[index]);
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

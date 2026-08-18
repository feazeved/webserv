#pragma once

#include <stdexcept>
#include <sys/socket.h>
#include <vector>
#include <cstring>
#include <cerrno>
#include <sys/epoll.h>
#include <csignal>
#include <iostream>

#include "HTTP.hpp"
#include "State.hpp"
#include "Connection.hpp"
#include "BlockVector.hpp"
#include "core.hpp"
#include "VirtualServer.hpp"

#define SERVER_INL(ret_type) ret_type inline Server::

class Server {
private:
	Server(const Server&);
	Server& operator=(const Server&);

public:
	static const usize s_maxEvents = 16;
	static const usize s_serverBlockSize = 8;
	static const usize s_connectionBlockSize = 32;

	BlockVector<VirtualServer, s_serverBlockSize, 16> servers;
	BlockVector<HTTP::Connection, s_connectionBlockSize, 64> connections;
	i32 epoll_fd;
	volatile bool running;

	Server(const std::vector<HTTP::ServerConfig>& configs)
		: epoll_fd(-1), running(false) {

		if (s_serverBlockSize == 0 || s_connectionBlockSize == 0)
			throw std::runtime_error("invalid BlockVector size");
		instance() = this;
		usize numServers = configs.size();
		while (servers.capacity() < numServers) {
			if (!servers.grow())
				throw std::bad_alloc();
		}
		for (usize i = 0; i < numServers; i++)
			servers[i].init(configs[i]);
		epoll_fd = epoll_create(1);
		if (epoll_fd == -1)
			throw std::runtime_error(std::strerror(errno));
	}

	~Server() {
		instance() = NULL;
		close(epoll_fd);
	}

	void run() {
		(void)signal(SIGINT, handle_signal);
		(void)signal(SIGPIPE, SIG_IGN);

		for (usize i = 0; i < servers.size(); i++)
			add_to_epoll(servers[i].listenFd, EPOLLIN, &servers[i]);

		struct epoll_event events[s_maxEvents];
		running = true;

		while (running) {
			i32 event_count = epoll_wait(epoll_fd, events, s_maxEvents, -1);

			if (event_count == -1) {
				if (errno == EINTR)
					continue;
				throw std::runtime_error(std::strerror(errno));
			}

			for (i32 i = 0; i < event_count; i++) {
				void* ptr = events[i].data.ptr;

				if (is_listening_socket(ptr))
					add_connection(static_cast<VirtualServer*>(ptr));
				else {
					HTTP::Connection* conn = static_cast<HTTP::Connection*>(ptr);
					isize ret = conn->dispatch(events[i].events);

					switch (ret) {
						case 0:
							close_connection(conn);
							break;
						case 2:
							modify_epoll_event(conn->clientFd, EPOLLOUT, conn);
							break;
						default:
							break;
					}
				}
			}
			broadcast_all_server_events();
		}
	}

	static Server*& instance() {
		static Server* inst = NULL;
		return inst;
	}

	static void handle_signal(int signum) {
		(void)signum;
		if (instance())
			instance()->running = false;
	}

	void mark_connection_writable(i32 fd, void* conn);
	void add_to_epoll(i32 fd, u32 events, void* ptr);
	void remove_from_epoll(i32 fd);
	void modify_epoll_event(i32 fd, u32 events, void* ptr);
	bool is_listening_socket(void* ptr);

	void add_connection(VirtualServer* server);
	void close_connection(HTTP::Connection* conn);
	void broadcast_all_server_events();

};

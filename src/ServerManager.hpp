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
#include "Server.hpp"
#include "Connection.hpp"
#include "core.hpp"
#include "BlockVector.hpp"

class ServerManager {
public:
	ServerManager(const std::vector<HTTP::ServerConfig>& configs)
		: epoll_fd(-1), running(false) {

		if (s_serverBlockSize == 0 || s_connectionBlockSize == 0)
			throw std::runtime_error("invalid BlockVector size");
		instance() = this;
		usize numServers = configs.size();
		while (servers.allocdSize() < numServers) {
			if (!servers.grow())
				throw std::bad_alloc();
		}
		for (usize i = 0; i < numServers; i++) {
			servers[i].init(configs[i]);
		}
		epoll_fd = epoll_create(1);
		if (epoll_fd == -1)
			throw std::runtime_error(std::strerror(errno));
	}

	~ServerManager() {
		instance() = NULL;
		close(epoll_fd);
	}

	void	run() {
		signal(SIGINT, handleSignal);
		signal(SIGPIPE, SIG_IGN);

		for (usize i = 0; i < servers.allocdSize() && servers[i].getFd() != -1; i++) {
			addToEpoll(servers[i].getFd(), EPOLLIN, &servers[i]);
		}

		struct epoll_event	events[s_maxEvents];
		running = true;

		while (running) {
			i32	event_count = epoll_wait(epoll_fd, events, s_maxEvents, -1);

			if (event_count == -1) {
				if (errno == EINTR)
					continue ;
				throw std::runtime_error(std::strerror(errno));
			}

			for (i32 i = 0; i < event_count; i++) {
				void*	ptr = events[i].data.ptr;

				if (isListeningSocket(ptr)) {
					handleNewConnection(static_cast<Server*>(ptr));
				} else {
					HTTP::Connection*	conn = static_cast<HTTP::Connection*>(ptr);
					HTTP::RequestAction	action = conn->request.handleEvent(events[i].events);

					switch (action) {
						case HTTP::REQ_CLOSE:
							closeConnection(conn);
							break ;
						case HTTP::REQ_WRITE:
							modifyEpollEvent(conn->request.fd, EPOLLOUT, conn);
							break ;
						case HTTP::REQ_CONTINUE:
							break ;
					}
				}
			}
		}
	}

private:
	static const usize											s_maxEvents = 16;
	static const usize											s_serverBlockSize = 8;
	static const usize											s_connectionBlockSize = 32;

	BlockVector<Server, s_serverBlockSize, 16>					servers;
	BlockVector<HTTP::Connection, s_connectionBlockSize, 64>	connections;
	i32															epoll_fd;
	volatile bool												running;


	static ServerManager*&	instance() {
		static ServerManager*	inst = NULL;
		return (inst);
	}

	static void	handleSignal(int signum) {
		(void)signum;
		if (instance())
			instance()->running = false;
	}

	void	addToEpoll(i32 fd, u32 events, void* ptr) {
		struct epoll_event	ev;
		ev.events = events;
		ev.data.ptr = ptr;
		if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1)
			throw std::runtime_error(std::strerror(errno));
	}

	void	removeFromEpoll(i32 fd) {
		epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
	}

	void	modifyEpollEvent(i32 fd, u32 events, void* ptr) {
		struct epoll_event	ev;
		ev.events = events;
		ev.data.ptr = ptr;
		if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev) == -1)
			std::cerr << "epoll_ctl MOD error: " << std::strerror(errno) << "\n";
	}

	bool	isListeningSocket(void* ptr) {
		for (usize i = 0; i < servers.allocdSize(); i++) {
			if (ptr == &servers[i])
				return (true);
		}
		return (false);
	}

	// function doesnt throw on error, only logs
	void	handleNewConnection(Server* server) {
		sockaddr_in	clientAddr;
		socklen_t	clientLen = sizeof(clientAddr);

		i32 clientFd = accept(server->getFd(), (sockaddr*)&clientAddr, &clientLen);
		if (clientFd == -1) {
			if (errno != EAGAIN && errno != EWOULDBLOCK)
				std::cerr << "accept error: " << std::strerror(errno) << "\n";
			return ;
		}

		if (fcntl(clientFd, F_SETFL, O_NONBLOCK) == -1) {
			std::cerr << "fcntl error: " << std::strerror(errno) << "\n";
			close(clientFd);
			return ;
		}

		usize	index;
		if (getFreeConnection(index) == false) {
			close(clientFd);
			throw std::bad_alloc();
		}
		connections[index].init(clientFd);
		addToEpoll(clientFd, EPOLLIN, &connections[index]);
	}

	void	closeConnection(HTTP::Connection* conn) {
		for (usize i = 0; i < connections.allocdSize(); i++) {
			if (&connections[i] == conn) {
				removeFromEpoll(conn->request.fd);
				break ;
			}
		}
	}

	bool	getFreeConnection(usize& index) {
		usize i;
		for (i = 0; i < connections.allocdSize(); i++) {
			if (connections[i].request.fd == -1) {
				index = i;
				return (true);
			}
		}
		if (connections.grow() == false)
			return (false);
		index = i;
		return (true);
	}

	// To prevent copying
	ServerManager(const ServerManager&);
	ServerManager& operator=(const ServerManager&);
};

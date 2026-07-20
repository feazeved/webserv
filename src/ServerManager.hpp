#pragma once

#include <map>
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
#include "core.hpp"

class ServerManager {
public:
	ServerManager(const std::vector<HTTP::ServerConfig>& configs)
		: epoll_fd(-1), running(false) {

		instance() = this;
		servers.reserve(configs.size());

		try {
			for (usize i = 0; i < configs.size(); i++) {
				servers.push_back(new Server(configs[i]));
			}
			epoll_fd = epoll_create(1);
			if (epoll_fd == -1)
				throw std::runtime_error(std::strerror(errno));
		} catch (...) {
			for (usize i = 0; i < servers.size(); i++) {
				delete servers[i];
			}
			servers.clear();
			throw ;
		}
	}

	~ServerManager() {
		instance() = NULL;
		for (usize i = 0; i < servers.size(); i++) {
			delete servers[i];
		}
		close(epoll_fd);
	}

	// maybe need to worry with signal?
	void	run() {
		signal(SIGINT, handleSignal);
		signal(SIGPIPE, SIG_IGN);

		for (usize i = 0; i < servers.size(); i++) {
			addToEpoll(servers[i]->getFd(), servers[i]);
		}

		struct epoll_event	events[s_max_events];
		running = true;

		while (running) {
			i32	event_count = epoll_wait(epoll_fd, events, s_max_events, -1); // 30000 -> timeout, need to check this

			if (event_count == -1) {
				if (errno == EINTR)
					continue ;
				throw std::runtime_error(std::strerror(errno));
			}

			for (i32 i = 0; i < event_count; i++) {
				void*	ptr = events[i].data.ptr;

				if (isListeningSocket(ptr)) {
					handleNewConnection(static_cast<Server*>(ptr));
				}
				else if (events[i].events & (EPOLLIN | EPOLLOUT)) {
					Connection*	conn = static_cast<Connection*>(ptr);

					conn->handleEvent(events[i].events);
				}
			}
		}
	}

private:
	std::vector<Server*>	servers;
	// vector de conexoes ?
	i32						epoll_fd;
	bool					running;

	static const usize		s_max_events = 16;

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

	bool	isListeningSocket(void* ptr) {
		for (usize i = 0; i < servers.size(); i++) {
			if (ptr == servers[i])
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
		}

		Connection*	conn = new Connection(clientFd, server->getConfig());

		connections.push_back(conn);

		addToEpoll(clientFd, EPOLLIN, conn);
	}

	// To prevent copying
	ServerManager(const ServerManager&);
	ServerManager& operator=(const ServerManager&);

// functions that I think Alex will need
public:
	// function doesnt throw on error, only logs
	void	modifyEpollEvent(i32 fd, u32 events, void* ptr) {
		struct epoll_event	ev;
		ev.events = events;
		ev.data.ptr = ptr;
		if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev) == -1)
			std::cerr << "epoll_ctl MOD error: " << std::strerror(errno) << "\n";
	}

	void	closeConnection(Connection* conn) {
		for (usize i = 0; i < connections.size(); i++) {
			if (connections[i] == conn) {
				removeFromEpoll(conn->getFd());
				delete connections[i];
				connections.erase(connections.begin() + i);

				break ;
			}
		}

	}
};

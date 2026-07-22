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
#include "Request.hpp"
#include "core.hpp"
#include "BlockVector.hpp"

class Request;

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
		for (usize i = 0; i < servers.size(); i++)
			delete servers[i];
		for (usize i = 0; i < requests.size(); i++)
			closeConnection(requests[i]);
		close(epoll_fd);
	}

	// maybe need to worry with signal?
	void	run() {
		signal(SIGINT, handleSignal);
		signal(SIGPIPE, SIG_IGN);

		for (usize i = 0; i < servers.size(); i++) {
			addToEpoll(servers[i]->getFd(), EPOLLIN, servers[i]);
		}

		struct epoll_event	events[s_max_events];
		running = true;

		while (running) {
			i32	event_count = epoll_wait(epoll_fd, events, s_max_events, -1);

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
					HTTP::Request*	req = static_cast<HTTP::Request*>(ptr);
					HTTP::RequestAction	action = req->handleEvent(events[i].events);

					switch (action) {
						case HTTP::REQ_CLOSE:
							closeConnection(req);
							break ;
						case HTTP::REQ_WRITE:
							modifyEpollEvent(req->fd, EPOLLOUT, req);
							break ;
						case HTTP::REQ_CONTINUE:
							break ;
					}
				}
			}
		}
	}

private:
	BlockVector<Server, 16, 16>				servers;
	BlockVector<HTTP::Connection, 64, 64>	connections;
	i32										epoll_fd;
	bool									running;

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
			return ;
		}

		if (fcntl(clientFd, F_SETFL, O_NONBLOCK) == -1) {
			std::cerr << "fcntl error: " << std::strerror(errno) << "\n";
			close(clientFd);
			return ;
		}
		HTTP::Request*	req = new HTTP::Request(clientFd);
		requests.push_back(req);
		addToEpoll(clientFd, EPOLLIN, req);
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

	void	closeConnection(HTTP::Request* req) {
		for (usize i = 0; i < requests.size(); i++) {
			if (requests[i] == req) {
				removeFromEpoll(req->fd);
				close(req->fd);
				delete requests[i];
				requests.erase(requests.begin() + static_cast<long>(i));
				break ;
			}
		}
	}
};

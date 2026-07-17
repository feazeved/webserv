#pragma once

#include <map>
#include <stdexcept>
#include <sys/socket.h>
#include <vector>
#include <cstring>
#include <cerrno>
#include <sys/epoll.h>

#include "HTTP.hpp"
#include "Server.hpp"
#include "core.hpp"

class ServerManager {
public:
	ServerManager(const std::vector<HTTP::ServerConfig>& configs) {
		try {
			servers.reserve(configs.size());
			for (usize i = 0; i < configs.size(); i++) {
				servers.push_back(new Server(configs[i]));
				listeningMap[servers[i]->getFd()] = servers[i];
			}
			epoll_fd = epoll_create(static_cast<i32>(servers.size()));
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
		for (usize i = 0; i < servers.size(); i++) {
			delete servers[i];
		}
		close(epoll_fd);
	}

	// maybe need to worry with signal?
	void	run() {
		for (usize i = 0; i < servers.size(); i++) {
			struct epoll_event	toAdd;
			toAdd.events = EPOLLIN;
			toAdd.data.fd = servers[i]->getFd();
			if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, toAdd.data.fd, &toAdd))
				throw std::runtime_error(std::strerror(errno));
		}

		struct epoll_event	events[s_max_events];
		i32					event_count;
		bool				running = true;

		while (running) {
			event_count = epoll_wait(epoll_fd, events, s_max_events, 30000); // 30000 -> timeout, need to check this
			for (i32 i = 0; i < event_count; i++) {
				if (listeningMap.count(events[i].data.fd) > 0)
				{
					// accept + add to epoll
				}
			}
		}
	}

private:
	std::map<int, Server*>	listeningMap;
	std::vector<Server*>	servers;
	i32						epoll_fd;

	static const usize		s_max_events = 16;
};

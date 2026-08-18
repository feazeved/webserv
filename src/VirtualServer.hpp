#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdexcept>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <netdb.h>

#include "HTTP.hpp"
#include "State.hpp"
#include "core.hpp"

static inline
sockaddr_in s_resolve_host_and_port(const std::string& host, i64 port) {
	std::string	hostToResolve = host.empty() ? "0.0.0.0" : host;

	addrinfo hints;
	MEMSET_INLINE(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	addrinfo*	result = NULL;
	int			status;

	status = getaddrinfo(hostToResolve.c_str(), NULL, &hints, &result);
	if (status != 0) {
		throw std::runtime_error("resolveHost '" + hostToResolve + "': " + gai_strerror(status));
	}

	sockaddr_in	addr;
	MEMSET_INLINE(&addr, 0, sizeof(addr));
	MEMCPY_INLINE(&addr, result->ai_addr, sizeof(sockaddr_in));

	addr.sin_port = htons(port);
	freeaddrinfo(result);

	return (addr);
}

class VirtualServer {
public:
	VirtualServer(const HTTP::ServerConfig& c) : listenFd(-1) {
		init(c);
	}

	VirtualServer() : listenFd(-1) {}

	~VirtualServer() {
		if (listenFd != -1)
			close(listenFd);
	}

	void init(const HTTP::ServerConfig& c) {
		config = c;
		listenFd = socket(AF_INET, SOCK_STREAM, 0);
		if (listenFd == -1)
			throw std::runtime_error(std::strerror(errno));

		i32	opt = 1;
		if (setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
			throw std::runtime_error(std::string("setsockopt: ") + std::strerror(errno));

		sockaddr_in	addr = s_resolve_host_and_port(config.host, config.port);

		if (bind(listenFd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1)
			throw std::runtime_error(std::string("bind: ") + std::strerror(errno));
		if (listen(listenFd, SOMAXCONN) == -1)
			throw std::runtime_error(std::string("listen: ") + std::strerror(errno));
		if (fcntl(listenFd, F_SETFL, O_NONBLOCK) == -1)
			throw std::runtime_error(std::string("fcntl: ") + std::strerror(errno));
	}

	HTTP::ServerConfig	config;
	i32					listenFd;
	Game::State			gameState;
};

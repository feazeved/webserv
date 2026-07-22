#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdexcept>
#include <cstring>
#include <cerrno>
#include <unistd.h>

#include "HTTP.hpp"
#include "core.hpp"

class Server {
public:
	Server(const HTTP::ServerConfig& c) : listenFd(-1) {
		init(c);
	}

	Server() : listenFd(-1) {}

	~Server() {
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
			closeAndThrow("setsockopt");

		sockaddr_in	addr;
		MEMSET_BUILTIN(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(config.port);

		std::string ip = resolveHost(config.host);
		if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) == -1)
			closeAndThrow("inet_pton");
		if (bind(listenFd, (sockaddr*)&addr, sizeof(addr)) == -1)
			closeAndThrow("bind");
		if (listen(listenFd, SOMAXCONN) == -1)
			closeAndThrow("listen");
		if (fcntl(listenFd, F_SETFL, O_NONBLOCK) == -1)
			closeAndThrow("fcntl");
	}

	i32	getFd() const { return (listenFd); }
	const HTTP::ServerConfig&	getConfig() const { return (config); }

private:
	HTTP::ServerConfig	config;
	i32					listenFd;


	void	closeAndThrow(const std::string& where) {
		close(listenFd);
		listenFd = -1;
		throw std::runtime_error(where + ": " + std::strerror(errno));
	}

	static std::string	resolveHost(const std::string& host) {
		if (host != "localhost")
			throw std::runtime_error("Server expected host 'localhost");
		return ("127.0.0.1");
	}
};

#pragma once

#include <sys/socket.h>
#include <stdexcept>
#include <cstring>
#include <cerrno>
#include <unistd.h>

#include "Http.hpp"

class Server {
public:
	Server(const Http::ServerConfig& c) : config(c), listenFd(-1) {
		listenFd = socket(AF_INET, SOCK_STREAM, 0);
		if (listenFd == -1)
			throw std::runtime_error(std::strerror(errno));
	}

	~Server() {
		close(listenFd);
	}


private:
	Http::ServerConfig	config;
	int					listenFd;
};

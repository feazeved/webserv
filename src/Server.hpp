#pragma once

#include "Http.hpp"

class Server {
public:
	Server(const Http::ServerConfig& c) : config(c), listenFd(-1) {
		boot();
	}

	~Server() {
		
	}

	void	boot() {
		(void)listenFd;
		
	}

private:
	Http::ServerConfig	config;
	int					listenFd;
};

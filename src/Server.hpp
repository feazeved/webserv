#pragma once

#include "Http.hpp"

class Server {
public:
	Server(const Http::ServerConfig& c);
	~Server();

	void	boot() const;

private:
	Http::ServerConfig	config;
	int					listenFd;
};

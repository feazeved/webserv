#pragma once

#include "ServerConfig.hpp"

class Server {
public:
	Server(const std::vector<ServerConfig>& s);
	~Server();

	void	run() const;

private:
	std::vector<ServerConfig>	servers;

	void	boot() const;
};

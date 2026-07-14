#pragma once

#include "ServerConfig.hpp"

class Server {
public:
	Server(const std::vector<ServerConfig>& s) : servers(s) {}

	void run() const;

private:
	std::vector<ServerConfig>	servers;
};

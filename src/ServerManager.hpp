#pragma once

#include <vector>

#include "Http.hpp"
#include "Server.hpp"

class ServerManager {
public:
	ServerManager(const std::vector<Http::ServerConfig>& configs);
	~ServerManager();

	void	run();

private:
	std::vector<Server*>	servers;
};

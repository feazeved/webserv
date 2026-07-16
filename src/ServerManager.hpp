#pragma once

#include <vector>

#include "Http.hpp"
#include "Server.hpp"

class ServerManager {
public:
	ServerManager(const std::vector<Http::ServerConfig>& configs) {
		servers.reserve(configs.size());
		for (std::size_t i = 0; i < configs.size(); i++) {
			Server* serv = NULL;
			servers.push_back(serv);
		}

		for (std::size_t i = 0; i < configs.size(); i++) {
			servers[i] = new Server(configs[i]);
		}
	}

	~ServerManager() {
		for (std::size_t i = 0; i < servers.size(); i++) {
			delete servers[i];
		}
	}

	void	run() {

	}

private:
	std::vector<Server*>	servers;
};

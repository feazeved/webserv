#pragma once

#include <vector>

#include "Http.hpp"
#include "Server.hpp"

class ServerManager {
public:
	ServerManager(const std::vector<Http::ServerConfig>& configs) {
		for (std::size_t i = 0; i < configs.size(); i++) {
			Server*	serv = new Server(configs[i]);

			serv->boot();
			servers.push_back(serv);
		}
	}

	~ServerManager() {

	}

	void	run() {

	}

private:
	std::vector<Server*>	servers;
};

#pragma once

#include <map>
#include <vector>
#include <iostream>

#include "Http.hpp"
#include "Server.hpp"

class ServerManager {
public:
	ServerManager(const std::vector<Http::ServerConfig>& configs) {
		try {
			servers.reserve(configs.size());
			for (std::size_t i = 0; i < configs.size(); i++) {
				servers.push_back(new Server(configs[i]));
				listeningMap[servers[i]->getFd()] = servers[i];
			}
		} catch (...) {
			for (std::size_t i = 0; i < servers.size(); i++) {
				delete servers[i];
			}
			servers.clear();
			throw ;
		}
	}

	~ServerManager() {
		for (std::size_t i = 0; i < servers.size(); i++) {
			delete servers[i];
		}
	}

	void	run() {
		std::cout << "teste\n";
	}

private:
	std::map<int, Server*>	listeningMap;
	std::vector<Server*>	servers;
};

#include <cstddef>

#include "ServerManager.hpp"
#include "Http.hpp"

ServerManager::ServerManager(const std::vector<Http::ServerConfig>& configs)
{

	for (std::size_t i = 0; i < configs.size(); i++) {
		Server*	serv = new Server(configs[i]);

		serv->boot();
		servers.push_back(serv);
	}

}

ServerManager::~ServerManager()
{
}

void	ServerManager::run()
{
}

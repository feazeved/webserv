#include <iostream>
#include <cstdlib>
#include <exception>

#include "ServerManager.hpp"
#include "parseConfig.hpp"

int	main(int argc, char** argv)
{
	if (argc != 2) {
		std::cerr << "Error: Usage -> ./webserv <config_file>\n";
		return (EXIT_FAILURE);
	}

	try {
		ServerManager	manager(parseConfig::parseConfig(argv[1]));

		manager.run();

		std::vector<ServerConfig>::iterator it = webserv.begin();
		for(;it != webserv.end(); it++)
		    std::cout << "Listen from port: " << it->getPort() << "\n";


	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << "\n";
		return (EXIT_FAILURE);
	}
}

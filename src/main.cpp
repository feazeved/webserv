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

	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << "\n";
		return (EXIT_FAILURE);
	}
}

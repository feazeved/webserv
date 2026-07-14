#include <iostream>
#include <cstdlib>
#include <exception>

#include "ServerConfig.hpp"
#include "parseConfig.hpp"

int	main(int argc, char** argv)
{
	if (argc != 2) {
		std::cerr << "Error: Usage -> ./webserv <config_file>\n";
		return (EXIT_FAILURE);
	}
	(void)argv;

	try {
		std::vector<ServerConfig>	webserv;
		webserv = parseConfig::parseConfig(argv[1]);

	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << "\n";
		return (EXIT_FAILURE);
	}
}

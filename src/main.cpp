#include <iostream>
#include <cstdlib>
#include <exception>

#include "ServerManager.hpp"
#include "parse_config.hpp"

static inline
ServerManager s_parse(char *str) {
	std::vector<HTTP::ServerConfig> res = parse_config::parse_config(str);


}

int	main(int argc, char** argv)
{
	if (argc != 2) {
		std::cerr << "Error: Usage -> ./webserv <config_file>\n";
		return EXIT_FAILURE;
	}

	try {
		ServerManager manager(parse_config::parse_config(argv[1]));

		manager.run();
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << "\n";
		return EXIT_FAILURE;
	}
}

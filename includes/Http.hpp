#pragma once

#include <string>
#include <vector>
#include <map>

namespace Http {

	struct Location {
		std::vector<std::string>	methods;
		std::string					path;
		std::string					root;
		bool						autoindex;

		Location() : autoindex(false) {}
	};

	struct ServerConfig {
		std::map<int, std::string>	errors;
		std::vector<Location>		locations;
		std::string					host;
		int							port;
		int							maxBodySize;

		ServerConfig() : host("localhost"), port(8080), maxBodySize(100000) {}
	};

}

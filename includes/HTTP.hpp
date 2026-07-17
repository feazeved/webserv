#pragma once

#include <string>
#include <vector>
#include <map>

namespace HTTP {

	struct Location {
		std::vector<std::string>	methods;
		std::string					path;
		std::string					root;
		std::string					index;
		std::string					upload_store;
		bool						autoindex;

		Location() : autoindex(false) {}
	};

	struct ServerConfig {
		std::map<long, std::string>	errors;
		std::vector<Location>		locations;
		std::string					host;
		long						port;
		long						maxBodySize;

		ServerConfig() : host("localhost"), port(-1), maxBodySize(-1) {}
	};

}

#pragma once

#include <string>
#include <vector>

#include "Location.hpp"

class ServerConfig{
	private:
		std::vector<Location>		locations;
		std::vector<std::string>	errors;
		std::string					host;
		int							port;
		int							maxBodySize;

	public:
};
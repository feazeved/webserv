#pragma once

#include <string>
#include <vector>

struct Location {
	std::string					path;
	std::string					root;
	std::vector<std::string>	methods;
};

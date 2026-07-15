#pragma once

#include <string>
#include <vector>
#include <sstream>

#include "Http.hpp"

namespace parseConfig {
	enum tokenType {
	    OPEN_BRACKET,
	    CLOSE_BRACKET,
	    SEMICOLON,
	    WORD
	};

	struct token {
	    tokenType type;
	    std::string value;
	};

	struct  Directive {
		std::string name;
		std::vector<std::string> args;
	};

	std::vector<Http::ServerConfig>	parseConfig(char *filePath);
}

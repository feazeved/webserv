#pragma once

#include <vector>
#include <sstream>


#include "ServerConfig.hpp"

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

	std::vector<ServerConfig> parseConfig(char *filePath);
}

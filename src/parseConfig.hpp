#pragma once

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

	std::vector<token> tokenizer(std::stringstream &config);
	std::vector<Http::ServerConfig>	parseConfig(char *filePath);
}

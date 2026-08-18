#pragma once

#include <string>
#include <vector>

#include "HTTP.hpp"

namespace parse_config {
// 

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

typedef std::vector<token>::const_iterator tokIter;

std::vector<HTTP::ServerConfig>	parse_config(char *filePath);

}

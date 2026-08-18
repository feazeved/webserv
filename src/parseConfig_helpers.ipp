#pragma once
#include "core.hpp"
#include <algorithm>
#include <cerrno>
#include <cstddef>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <cstdlib>
#include <sstream>

#include "parseConfig.hpp"
#include "HTTP.hpp"
#include "Server.hpp"
#include "Status.hpp"
#include "core_builtins.ipp"

namespace parseConfig {

static inline
void s_match(tokIter &cursor, const std::string &value) {
	if (cursor->value != value)
		throw std::runtime_error("Unexpected token");
	cursor++;
}

static inline
void s_advance(tokIter &cursor, tokIter &end) {
	if (cursor == end)
		throw std::runtime_error("Invalid read");
	cursor++;
}

static inline
long s_strtol(std::string& str) {
	const char *sptr = str.c_str();
	char *eptr = NULL;

	long ret = std::strtol(sptr, &eptr, 10);

	if (*eptr || errno == ERANGE)
		throw std::runtime_error("Invalid directive");
	return (ret);
}

void tokenizer_dump(std::vector<token> &tokens) {
	tokIter it =  tokens.begin();
	std::cout << "---Print tokens---\n\n";
	for (;it != tokens.end(); it++)
	{
		std::string tp;
		switch (it->type) {
			case OPEN_BRACKET: tp = "open bracket"; break;
			case CLOSE_BRACKET: tp = "close bracket"; break;
			case SEMICOLON: tp = "semicolon"; break;
			case WORD: tp = "word"; break;
			// TODO: default?
		}
		std::cout << tp ;
		if (tp == "word")
			std::cout << "(" << it->value << ")";
		std::cout << "\n";
	}
}

void config_dump(std::vector<HTTP::ServerConfig> &config) {
	std::vector<HTTP::ServerConfig>::iterator it =  config.begin();
	std::cout << "---Print Config---\n\n";
	for (; it != config.end(); it++)
	{
		std::cout << "SERVER\n";
		std::cout << "\tlisten: " << (*it).port << "\n";
		std::cout << "\thost: " << (*it).host << "\n";
		std::cout << "\tmax_body_size: " << (*it).maxBodySize << "\n";
		std::cout << "\n";

		std::vector<HTTP::Location>::iterator itl =  (*it).locations.begin();
		for (; itl != (*it).locations.end(); itl++)
		{
			std::cout << "\tLOCATION " << (*itl).path << "\n";
			std::cout << "\t\troot: " << (*itl).root << "\n";
			std::cout << "\t\tindex: " << (*itl).index << "\n";
			std::cout << "\t\tupload_store: " << (*itl).upload_store << "\n";
			std::cout << "\t\tautoindex: " << ((*itl).autoindex ? "on " : "off ") << "\n";
			std::cout << "\n";
		}
	}

}

}
#pragma once
#include "core.hpp"

#include "stdlib.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include "HTTP.hpp"

namespace HTTP {
namespace Parse {
// 
typedef std::vector<Token>::const_iterator tokIter;

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

static inline
void s_set_methods(std::vector<std::string> &methods, HTTP::Location &location) {
	std::vector<std::string>::iterator it = methods.begin();

	for (; it != methods.end(); it++)
	{
		if (it->size() == 3 && MEMCMP(it->c_str(), "GET", 3) == 0)
			location.methods |= HTTP::Mode::GET;
		else if (it->size() == 4 && MEMCMP(it->c_str(), "POST", 4) == 0)
			location.methods |= HTTP::Mode::POST;
		else if (it->size() == 6 && MEMCMP(it->c_str(), "DELETE", 6) == 0)
			location.methods |= HTTP::Mode::DELETE;
		else
			throw std::runtime_error("Invalid method");
	}
}

static inline
usize s_find_scope_end(tokIter &begin, tokIter &end) {
	tokIter it = begin;
	bool startedCount = false;
	int braces = 0;
	usize distance = 0;

	while (it != end) {
		if (it->type == Token::OPEN_BRACKET) {
			startedCount = true;
			braces++;
		}
		else if (it->type == Token::CLOSE_BRACKET) {
			startedCount = true;
			braces--;
		}
		if (!braces && startedCount)
			break;
		it++;
		distance++;
	}
	return distance;
}

void tokenizer_dump(std::vector<Token> &tokens) {
	tokIter it =  tokens.begin();
	std::cout << "---Print tokens---\n\n";
	for (;it != tokens.end(); it++)
	{
		std::string tp;
		switch (it->type) {
			case Token::OPEN_BRACKET: tp = "open bracket"; break;
			case Token::CLOSE_BRACKET: tp = "close bracket"; break;
			case Token::SEMICOLON: tp = "semicolon"; break;
			case Token::WORD: tp = "word"; break;
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
}

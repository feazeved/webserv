#pragma once
#include "core.hpp"

#include "stdlib.h"
#include <iostream>
#include <string>
#include <vector>
#include "HTTP.hpp"

namespace HTTP {
namespace Parse {

typedef std::vector<Token>::const_iterator tokIter;

// temporary solution
static inline
int s_cleanup(char *filePath) {
	static char* ptr = NULL;

	if (filePath == NULL) {
		delete[] ptr;
		return 1;
	}
	else {
		ptr = filePath;
		return 0;
	}
}

static inline
void s_match(tokIter &cursor, const std::string &value) {
	if (cursor->value != value)
		PERR_EXIT(s_cleanup(NULL), "Error: Unexpected token");
	cursor++;
}

static inline
void s_advance(tokIter &cursor, tokIter &end) {
	if (cursor == end)
		PERR_EXIT(s_cleanup(NULL), "Error: Invalid read");
	cursor++;
}

static inline
long s_strtol(std::string& str) {
	const char *sptr = str.c_str();
	char *eptr = NULL;

	long ret = std::strtol(sptr, &eptr, 10);

	if (*eptr || errno == ERANGE)
		PERR_EXIT(s_cleanup(NULL), "Error: Invalid directive");
	return ret;
}

static inline
void s_set_methods(std::vector<std::string> &methods, HTTP::Location &location) {
	std::vector<std::string>::iterator it = methods.begin();

	for (; it != methods.end(); it++)
	{
		if (MEMCMP_INLINE(it->c_str(), "GET") == 0)
			location.methods |= HTTP::Mode::GET;
		else if (MEMCMP_INLINE(it->c_str(), "POST") == 0)
			location.methods |= HTTP::Mode::POST;
		else if (MEMCMP_INLINE(it->c_str(), "DELETE") == 0)
			location.methods |= HTTP::Mode::DELETE;
		else
			PERR_EXIT(s_cleanup(NULL), "Error: Invalid method");
	}
}

static inline
usize s_count_tokens(const char *str) {
	usize tokenCount = 0;
	while (true) {
		while (IS_SPACE(*str))
			str++;
		if (*str == 0)
			return tokenCount;
		tokenCount++;
		while (*str > 32)
			str++;
	}
	return tokenCount;
}

static inline
usize s_count_servers(const char *str, usize length) {
	const char *end = str + length;
	usize serverCount = 0;
	isize pdepth = 0;

	while (str < end) {
		while (IS_SPACE(*str))
			str++;
		if (MEMCMP_INLINE(str, "server") != 0) {
			if (*str == 0)
				return serverCount;
			PERR_EXIT(s_cleanup(NULL), "Error: Invalid config");
		}
		str += 6;
		while (IS_SPACE(*str))
			str++;
		pdepth = (*str == '{') ? 1 : -1;
		str++;
		for (; str < end && pdepth > 0; str++) {
			for (; *str != '}'; str++)
				pdepth += *str == '{';
			if (str < end)
				pdepth--;
		}
		if (pdepth != 0)
			PERR_EXIT(s_cleanup(NULL), "Error: Invalid config");
		serverCount++;
	}
	return serverCount;
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

// 
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

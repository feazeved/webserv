#pragma once

#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <sstream>
#include "HTTP.hpp"
#include "parse_config_helpers.ipp"

namespace HTTP {
namespace Parse {
//

static inline
isize s_get_word(char* &fileBuffer, std::string &word) {
	while (IS_SPACE(*fileBuffer))
		fileBuffer++;
	char *ostr = fileBuffer;
	if (*fileBuffer == 0)
		return 1;
	while (*fileBuffer > 32)
		fileBuffer++;
	usize length = (usize) (fileBuffer - ostr);
	if (length > 256)
		PERR_RETURN(-1, "Error: field is too large");
	word.assign(ostr, length);
	return 0;
}

static inline
usize s_count_tokens(const char *str) {
	usize wordCount = 0;
	while (true) {
		while (IS_SPACE(*str))
			str++;
		if (*str == 0)
			return wordCount;
		wordCount++;
		while (*str > 32)
			str++;
	}
	return wordCount;
}

static inline
isize s_match_delimiter(std::string &word, usize &delimPos, isize &braces, Token &token) {
	char delimiter = word[delimPos];

	if (delimPos + 1 != word.size())
		PERR_RETURN(-1, "Syntax error");
	switch (delimiter) {
		case '{' :
			token.type = Token::OPEN_BRACKET;
			token.value = '{';
			braces++;
			break;
		case '}' :
			token.type = Token::CLOSE_BRACKET;
			token.value = '}';
			braces--;
			if (braces < 0)
				PERR_RETURN(-1, "Error: Extraneous closing brace ('}')");
			break;
		case ';' :
			if (token.type == Token::SEMICOLON)
				PERR_RETURN(-1, "Error: Extraneous semicolon (';')");
			token.type = Token::SEMICOLON;
			token.value = ';';
			break;
		default: PERR_RETURN(-1, "Error: Invalid delimiter"); break;
	}
	return 0;
}

isize tokenizer(char *fileBuffer, std::vector<Token> &ret) {
	std::string word;
	Token token;
	isize braces = 0;
	word.reserve(256);
	isize rvalue;

	while ((rvalue = s_get_word(fileBuffer, word)) == 0) {
		usize delimPos = word.find_first_of("{};");
		if (delimPos == std::string::npos) {
			token.type = Token::WORD;
			token.value = word;
		}
		else {
			if (delimPos != 0) {
				token.type = Token::WORD;
				token.value = word.substr(0, delimPos);
				ret.push_back(token);
			}
			if (s_match_delimiter(word, delimPos, braces, token) == -1)
				return -1;
		}
		ret.push_back(token);
	}
	if (rvalue == -1)
		return -1;
	if (braces != 0)
		PERR_RETURN(-1, "Expected '}' to match previous '{'");
	return 0;
}

//
}	// Namespace Parse
}	// Namespace HTTP
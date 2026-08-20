#pragma once

#include <string>
#include <vector>
#include "HTTP.hpp"
#include "parse_helpers.ipp"

namespace HTTP {
namespace Parse {
//

static inline
usize s_get_word(char* &fileBuffer, std::string &word) {
	while (IS_SPACE(*fileBuffer))
		fileBuffer++;
	char *ostr = fileBuffer;
	while (*fileBuffer > 32)
		fileBuffer++;

	usize length = (usize) (fileBuffer - ostr);
	if (length > 256)
		PERR_EXIT(s_cleanup(NULL), "Error: Field is too large");
	else if (length == 0 && *fileBuffer != 0)
		PERR_EXIT(s_cleanup(NULL), "Error: Invalid character");

	word.assign(ostr, length);
	return length;
}

static inline
void s_match_delimiter(std::string &word, usize &delimPos, isize &braces, Token &token) {
	char delimiter = word[delimPos];

	if (delimPos + 1 != word.size())
		PERR_EXIT(s_cleanup(NULL), "Error: Syntax error");
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
				PERR_EXIT(s_cleanup(NULL), "Error: Extraneous closing brace ('}')");
			break;
		case ';' :
			if (token.type == Token::SEMICOLON)
				PERR_EXIT(s_cleanup(NULL), "Error: Extraneous semicolon (';')");
			token.type = Token::SEMICOLON;
			token.value = ';';
			break;
		default:
			PERR_EXIT(s_cleanup(NULL), "Error: Invalid delimiter");
			break;
	}
}

std::vector<Token> tokenizer(char *fileBuffer) {
	std::string word;
	std::vector<Token> tokVector;
	Token token;
	isize braces = 0;

	word.reserve(256);
	tokVector.reserve(s_count_tokens(fileBuffer));

	while (s_get_word(fileBuffer, word) > 0) {
		usize delimPos = word.find_first_of("{};");
		if (delimPos == std::string::npos) {
			token.type = Token::WORD;
			token.value = word;
		}
		else {
			if (delimPos != 0) {
				token.type = Token::WORD;
				token.value = word.substr(0, delimPos);
				tokVector.push_back(token);
			}
			s_match_delimiter(word, delimPos, braces, token);
		}
		tokVector.push_back(token);
	}
	if (braces != 0)
		PERR_EXIT(s_cleanup(NULL), "Error: Expected '}' to match previous '{'");
	return tokVector;
}

//
}	// Namespace Parse
}	// Namespace HTTP
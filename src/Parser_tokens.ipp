#pragma once

#include <string>
#include <cstring>
#include <vector>
#include "HTTP.hpp"
#include "Parser.hpp"

namespace HTTP {
//

PARSER_INL
(usize) get_next_word(char* &ostr) {
	while (IS_SPACE(*ostr))
		ostr++;
	char *str = ostr;
	while (*str > 32)
		str++;

	usize length = (usize) (str - ostr);
	if (length > 256)
		PERR_EXIT(cleanup(), "Error: Field is too large");
	else if (length == 0 && *str != 0)
		PERR_EXIT(cleanup(), "Error: Invalid character");
	return length;
}

PARSER_INL
(Parser::Token) match_delimiter(char *ptr, usize delimPos, isize &braces) {
	Token token;
	char delimiter = ptr[delimPos];

	token.value = ptr + delimPos;
	switch (delimiter) {
		case '{' :
			token.type = Token::OPEN_BRACKET;
			braces++;
			break;
		case '}' :
			token.type = Token::CLOSE_BRACKET;
			braces--;
			if (braces < 0)
				PERR_EXIT(cleanup(), "Error: Extraneous closing brace ('}')");
			break;
		case ';' :
			if (token.type == Token::SEMICOLON)
				PERR_EXIT(cleanup(), "Error: Extraneous semicolon (';')");
			token.type = Token::SEMICOLON;
			break;
		default:
			PERR_EXIT(cleanup(), "Error: Invalid delimiter");
			break;
	}
	return token;
}

PARSER_INL
(std::vector<Parser::Token>) tokenize() {
	std::vector<Token> tokVector;
	Token token;
	usize length;
	isize braces = 0;
	char *ptr = fileBuffer;

	tokVector.reserve(s_count_tokens(fileBuffer));

	while ((length = get_next_word(ptr)) > 0) {
		char* delimPtr = std::strpbrk(ptr, "{};");
		usize delimPos = (delimPtr == NULL) ? SIZE_MAX : (usize)(delimPtr - ptr);
		if (delimPos == SIZE_MAX) {
			token.type = Token::WORD;
			token.value = ptr;
		}
		else if (delimPos + 1 != length)
			PERR_EXIT(cleanup(), "Error: Syntax error");
		else {
			if (delimPos != 0) {
				token.type = Token::WORD;
				token.value = fileBuffer;
				tokVector.push_back(token);
			}
			token = match_delimiter(ptr, delimPos, braces);
		}
		ptr += length;
		tokVector.push_back(token);
	}
	if (braces != 0)
		PERR_EXIT(cleanup(), "Error: Expected '}' to match previous '{'");
	return tokVector;
}

//
}	// Namespace HTTP
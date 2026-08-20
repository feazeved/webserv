#pragma once

#include <string>
#include <vector>
#include "HTTP.hpp"
#include "Parser.hpp"

namespace HTTP {
//

PARSER_INL
(usize) get_word(char* &str, std::string &word) {
	while (IS_SPACE(*str))
		str++;
	char *ostr = str;
	while (*str > 32)
		str++;

	usize length = (usize) (str - ostr);
	if (length > 256)
		PERR_EXIT(cleanup(), "Error: Field is too large");
	else if (length == 0 && *str != 0)
		PERR_EXIT(cleanup(), "Error: Invalid character");

	word.assign(ostr, length);
	return length;
}

PARSER_INL
(void) match_delimiter(std::string &word, usize &delimPos, isize &braces, Token &token) {
	char delimiter = word[delimPos];

	if (delimPos + 1 != word.size())
		PERR_EXIT(cleanup(), "Error: Syntax error");
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
				PERR_EXIT(cleanup(), "Error: Extraneous closing brace ('}')");
			break;
		case ';' :
			if (token.type == Token::SEMICOLON)
				PERR_EXIT(cleanup(), "Error: Extraneous semicolon (';')");
			token.type = Token::SEMICOLON;
			token.value = ';';
			break;
		default:
			PERR_EXIT(cleanup(), "Error: Invalid delimiter");
			break;
	}
}

PARSER_INL
(std::vector<Parser::Token>) tokenizer(char *fileBuffer) {
	std::string word;
	std::vector<Token> tokVector;
	Token token;
	isize braces = 0;

	word.reserve(256);
	tokVector.reserve(s_count_tokens(fileBuffer));

	while (get_word(fileBuffer, word) > 0) {
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
			match_delimiter(word, delimPos, braces, token);
		}
		tokVector.push_back(token);
	}
	if (braces != 0)
		PERR_EXIT(cleanup(), "Error: Expected '}' to match previous '{'");
	return tokVector;
}

//
}	// Namespace HTTP
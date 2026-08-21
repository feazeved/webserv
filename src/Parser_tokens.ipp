#pragma once

#include "HTTP.hpp"
#include "Parser.hpp"

namespace HTTP {
//

PARSER_INL
(usize) get_next_word(char* &ostr) {
	while (IS_SPACE(*ostr))
		ostr++;
	char *str = ostr;
	while (*str > 32 && !s_is_config_delimiter(*str))
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

	token.value = StringView(1, (u32)(ptr + delimPos - (char*)Arena::data));
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
			token.type = Token::SEMICOLON;
			break;
		default:
			PERR_EXIT(cleanup(), "Error: Invalid delimiter");
			break;
	}
	return token;
}

PARSER_INL
(Array<Parser::Token>) tokenize() {
	Array<Token> tokArray;
	Token token;
	usize length;
	isize braces = 0;
	usize tokenIndex = 0;

	if (tokArray.alloc((u32)s_count_tokens(getPtr())) == true)
		_exit(1);
	char *optr = getPtr();
	char *ptr = optr;

	while (true) {
		while (IS_SPACE(*ptr))
			ptr++;
		if (*ptr == 0)
			break;
		if (s_is_config_delimiter(*ptr)) {
			token = match_delimiter(ptr, 0, braces);
			ptr++;
		}
		else {
			length = get_next_word(ptr);
			token.type = Token::WORD;
			token.value = StringView((u32)length, (u32)(ptr - optr));
			ptr += length;
		}
		tokArray[tokenIndex++] = token;
	}
	if (braces != 0)
		PERR_EXIT(cleanup(), "Error: Expected '}' to match previous '{'");
	return tokArray;
}

//
}	// Namespace HTTP

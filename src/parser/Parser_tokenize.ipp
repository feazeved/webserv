#pragma once
#include "Parser.hpp"

static inline
usize s_count_tokens(const char *str) {
	usize tokenCount = 0;
	while (true) {
		while (IS_SPACE(*str))
			str++;
		if (*str == 0)
			return tokenCount;
		tokenCount++;
		if (s_is_config_delimiter(*str))
			str++;
		else {
			const char *word = str;
			while ((u8)*str > 32 && !s_is_config_delimiter(*str))
				str++;
			if (str == word)
				str++;
		}
	}
	return tokenCount;
}

static inline
usize s_get_next_word(char* &ostr) {
	while (IS_SPACE(*ostr))
		ostr++;
	char *str = ostr;
	while ((u8)*str > 32 && !s_is_config_delimiter(*str))
		str++;

	usize length = (usize) (str - ostr);
	if (length == 0 && *str != 0)
		PERR_EXIT(1, "Error: Invalid character");
	return length;
}

static inline 
Parser::Token s_match_delimiter(char *ptr, usize delimPos, isize &braces) {
	Parser::Token token;
	char delimiter = ptr[delimPos];

	token.value = StringView32(1, (u32)(ptr + delimPos - (char*)Arena::pool.A));
	switch (delimiter) {
		case '{' :
			token.type = Parser::Token::OPEN_BRACKET;
			braces++;
			break;
		case '}' :
			token.type = Parser::Token::CLOSE_BRACKET;
			braces--;
			if (braces < 0)
				PERR_EXIT(1, "Error: Extraneous closing brace ('}')");
			break;
		case ';' :
			token.type = Parser::Token::SEMICOLON;
			break;
		default:
			PERR_EXIT(1, "Error: Invalid delimiter");
			break;
	}
	return token;
}

static inline
usize s_count_servers(const char *str, usize length) {
	const char *ostr;
	const char *end = str + length;
	usize serverCount = 0;
	isize pdepth = 0;

	while (str < end) {
		while (IS_SPACE(*str))
			str++;
		if (STRCMP(str, "server") != 0) {
			if (*str == 0)
				return serverCount;
			return SIZE_MAX;
		}
		str += 6;
		while (IS_SPACE(*str))
			str++;
		pdepth = (*str == '{') ? 1 : -1;
		str++;
		ostr = str;
		for (; str < end && pdepth > 0; str++) {
			for (; *str != '}'; str++)
				pdepth += *str == '{';
			if (str < end)
				pdepth--;
		}
		if (pdepth != 0)
			return SIZE_MAX;
		if (str - ostr > MAX_SERVER_BLOCK_SIZE)
			return SIZE_MAX;
		serverCount++;
	}
	return serverCount;
}

static inline
void s_strip_comments(char *ptr, usize fileSize) {
	static const char sentinels[] = "\0{};localhost";	// Also appends sentinels to the string

	for (usize index = 0; index < fileSize; index++) {
		if (ptr[index] == '#') {
			while (index < fileSize && ptr[index] != '\n')
				ptr[index++] = ' ';
		}
	}
	MEMCPY_INLINE(ptr + fileSize, sentinels, sizeof(sentinels));
}


PARSER_INL
(Array32<Parser::Token>) tokenize() {
	Array32<Token> tokArray;
	Token token;
	usize length;
	isize braces = 0;
	usize tokenIndex = 0;
	char *ptr = (char*) Arena::mptr(fileOffset);

	s_strip_comments(ptr, fileSize);
	serverCount = s_count_servers(ptr, fileSize);
	if (serverCount == 0 || serverCount > MAX_VIRTUAL_SERVERS)
		PERR_EXIT(1, "Error: Invalid config");

	if (tokArray.alloc_a((u32)s_count_tokens(ptr)) == true)
		_exit(1);

	while (true) {
		while (IS_SPACE(*ptr))
			ptr++;
		if (*ptr == 0)
			break;
		if (s_is_config_delimiter(*ptr)) {
			token = s_match_delimiter(ptr, 0, braces);
			ptr++;
		}
		else {
			length = s_get_next_word(ptr);
			token.type = Token::WORD;
			token.value = StringView32((u32)length, (u32)(ptr - (char*)Arena::pool.A));
			ptr += length;
		}
		tokArray[tokenIndex++] = token;
	}
	if (braces != 0)
		PERR_EXIT(1, "Error: Expected '}' to match previous '{'");
	for (u32 index = 0; index < tokArray.count; index++) {
		if (tokArray[index].type == Token::WORD) {
			StringView32 &word = tokArray[index].value;
			word.mptr()[word.length] = '\0';
		}
	}
	return tokArray;
}

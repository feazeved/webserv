#pragma once

#include "HTTP.hpp"
#include "Server.hpp"
#include <sys/stat.h>

namespace HTTP {
//

SERVER_INL
(usize) get_next_word(char* &ostr) {
	while (IS_SPACE(*ostr))
		ostr++;
	char *str = ostr;
	while ((u8)*str > 32 && !s_is_config_delimiter(*str))
		str++;

	usize length = (usize) (str - ostr);
	if (length == 0 && *str != 0)
		PERR_EXIT(cleanup(), "Error: Invalid character");
	return length;
}

SERVER_INL
(Server::Token) match_delimiter(char *ptr, usize delimPos, isize &braces) {
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

SERVER_INL
(Array32<Server::Token>) tokenize() {
	Array32<Token> tokArray;
	Token token;
	usize length;
	isize braces = 0;
	usize tokenIndex = 0;

	if (tokArray.alloc((u32)s_count_tokens(get_ptr())) == true)
		_exit(1);
	char *ptr = get_ptr();

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
			token.value = StringView((u32)length, (u32)(ptr - (char*)Arena::data));
			ptr += length;
		}
		tokArray[tokenIndex++] = token;
	}
	if (braces != 0)
		PERR_EXIT(cleanup(), "Error: Expected '}' to match previous '{'");
	for (u32 index = 0; index < tokArray.count; index++) {
		if (tokArray[index].type == Token::WORD) {
			StringView &word = tokArray[index].value;
			((char*)Arena::data + word.offset)[word.length] = '\0';
		}
	}
	return tokArray;
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

SERVER_INL
(void) read_whole_file(const char* filePath) {
	int fd = open(filePath, O_RDONLY);
	if (fd == -1)
		PERR_EXIT(1, "Error: Failed to open file");

	struct stat st;
	fileSize = (usize) st.st_size;
	usize allocSize = ALIGN_UP(fileSize + 63, (usize)64);	// Pads with at least 64 bytes

	if (fstat(fd, &st) == -1 || st.st_size < 16 || allocSize > MAX_FILE_SIZE) {
		close(fd);
		PERR_EXIT(1, "Error: Invalid file");
	}

	fileOffset = Arena::alloc_index(allocSize);

	char* ptr = get_ptr();
	usize curBytes = 0;
	while (curBytes < fileSize) {
		usize bytesRemaining = fileSize - curBytes;
		isize bytesRead = read(fd, ptr + curBytes, MIN(bytesRemaining, ATOMIC_IOSIZE));
		if (bytesRead <= 0) {
			close(fd);
			PERR_EXIT(1, "Error: Read failure");
		}
		curBytes += (usize) bytesRead;
	}
	close(fd);
	s_strip_comments(ptr, fileSize);
}

//
}	// Namespace HTTP

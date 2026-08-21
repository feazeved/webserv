#pragma once

#include "HTTP.hpp"
#include "Server.hpp"

namespace HTTP {
//

SERVER_INL
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

SERVER_INL
(void) read_whole_file(const char* filePath) {
	int fd = open(filePath, O_RDONLY);
	if (fd == -1)
		PERR_EXIT(1, "Error: Failed to open file");

	struct stat st;
	if (fstat(fd, &st) == -1 || st.st_size < 16 || (u64)st.st_size > (u64)UINT32_MAX - 127) {
		close(fd);
		PERR_EXIT(1, "Error: Invalid file");
	}

	fileSize = (usize) st.st_size;
	usize allocSize = ALIGN_UP(fileSize + 63, (usize)64);	// Pads with at least 64 bytes

	fileOffset = Arena::alloc_index(allocSize);
	if (fileOffset == UINT32_MAX) {
		close(fd);
		_exit(1);
	}

	char* ptr = getPtr();
	usize curBytes = 0;
	while (curBytes < fileSize) {
		usize bytesRemaining = fileSize - curBytes;
		isize bytesRead = read(fd, ptr + curBytes, MIN(bytesRemaining, ATOMIC_IOSIZE));
		if (bytesRead <= 0) {
			close(fd);
			Arena::clear();
			PERR_EXIT(1, "Error: Read failure");
		}
		curBytes += (usize) bytesRead;
	}
	close(fd);
	ptr[fileSize] = 0;
	ptr[fileSize + 1] = '{';
	ptr[fileSize + 2] = '}';
	ptr[fileSize + 3] = ';';
	MEMCPY_INLINE(ptr + fileSize + 4, "localhost", sizeof("localhost"));
}

//
}	// Namespace HTTP

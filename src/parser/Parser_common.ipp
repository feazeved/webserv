#pragma once
#include "Parser.hpp"

static inline
bool s_length_check(usize length) {
	return length == 0 || length >= MAX_PATH_SIZE;
}

// Exact
usize s_strtol10(const char *str, usize length) {
	usize value = 0;
	usize digit = 0;
	const char *end = str + length;

	while (str < end) {
		digit = (usize)(*str - '0');
		if (digit > 9 || value > ((SIZE_MAX - 9) / 10))
			break;
		value = value * 10 + digit;
		str++;
	}
	if (str != end)
		return SIZE_MAX;
	return value;
}

PARSER_INL
(bool) s_read_whole_file(Arena &arena, const char *filePath, Span &file, usize padSize, usize minSize, usize maxSize) {
	struct stat st;
	if (stat(filePath, &st) == -1 || (usize)st.st_size < minSize || (usize)st.st_size >= maxSize)
		PERR_RETURN(1, "Error: Invalid file");

	int fd = open(filePath, O_RDONLY);
	if (fd == -1)
		PERR_RETURN(1, "Error: Failed to open file");

	const usize fileSize = (usize)st.st_size;
	const u32 fileOffset = arena.alloc(fileSize, 1 + padSize);
	if (fileOffset == UINT32_MAX) {
		close(fd);
		PERR_RETURN(1, "Error: Out of memory");
	}

	u8* ptr = arena.mptr(fileOffset);
	usize curBytes = 0;
	while (curBytes < fileSize) {
		usize bytesRemaining = fileSize - curBytes;
		isize bytesRead = read(fd, ptr + curBytes, MIN(bytesRemaining, ATOMIC_IOSIZE));
		if (bytesRead <= 0) {
			close(fd);
			PERR_RETURN(1, "Error: Read failure");
		}
		curBytes += (usize) bytesRead;
	}
	close(fd);
	ptr[fileSize] = '\0';
	file.ptr = (char*)ptr;
	file.length = fileSize;
	return 0;
}

PARSER_INL
(Parser::Directive) s_build_directive(Arena &arena, Array<Token> &tokArray) {
	Directive dir;
	if (tokArray[0].type != Token::WORD)
		PERR_EXIT(1, "Error: Unexpected token");
	dir.name = tokArray[0].value;
	tokArray.ptr++;
	Token *argumentStart = tokArray.ptr;
	while (tokArray[0].type == Token::WORD)
		tokArray.ptr++;
	if (tokArray[0].type != Token::SEMICOLON)
		PERR_EXIT(1, "Error: Unexpected token");

	const usize argumentCount = (usize)(tokArray.ptr - argumentStart);
	dir.args = arena.alloc_array<Span>(argumentCount);
	if (argumentCount != 0 && dir.args.ptr == NULL)
		std::exit(1);
	for (usize index = 0; index < dir.args.count; index++)
		dir.args[index] = argumentStart[index].value;
	tokArray.ptr++;
	return dir;
}

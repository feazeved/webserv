#pragma once
#include "Parser.hpp"

static inline
bool s_is_config_delimiter(char value) {
	return value == '{' || value == '}' || value == ';';
}

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

// PARSER_INL
// (bool) s_read_whole_file2(char *ptr, const char *filePath, usize fileSize) {
// 	int fd = open(filePath, O_RDONLY);
// 	if (fd == -1)
// 		PERR_RETURN(1, "Error: Failed to open file");

// 	usize curBytes = 0;
// 	while (curBytes < fileSize) {
// 		usize bytesRemaining = fileSize - curBytes;
// 		isize bytesRead = read(fd, ptr + curBytes, MIN(bytesRemaining, ATOMIC_IOSIZE));
// 		if (bytesRead <= 0) {
// 			close(fd);
// 			PERR_RETURN(1, "Error: Read failure");
// 		}
// 		curBytes += (usize) bytesRead;
// 	}
// 	close(fd);
// 	ptr[fileSize] = '\0';
// 	return 0;
// }

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
(Parser::Directive) s_build_directive(Arena &arena, const Array<Token> &tokens, usize &cursor, usize end) {
	Directive dir;
	if (cursor == end || tokens[cursor].type != Token::WORD)
		PERR_EXIT(1, "Error: Unexpected token");
	dir.name = tokens[cursor].value;
	cursor++;
	usize argumentStart = cursor;
	while (cursor != end && tokens[cursor].type == Token::WORD)
		cursor++;
	if (cursor == end || tokens[cursor].type != Token::SEMICOLON)
		PERR_EXIT(1, "Error: Unexpected token");

	const usize argumentCount = cursor - argumentStart;
	dir.args = arena.alloc_array<Span>(argumentCount);
	if (argumentCount != 0 && dir.args.ptr == NULL)
		std::exit(1);
	for (usize index = 0; index < dir.args.count; index++)
		dir.args[index] = tokens[argumentStart + index].value;
	return dir;
}

PARSER_INL
(usize) s_find_scope_end(const Array<Token> &tokens, usize begin, usize end) {
	usize it = begin;
	bool startedCount = false;
	int braces = 0;
	usize distance = 0;

	while (it != end) {
		if (tokens[it].type == Token::OPEN_BRACKET) {
			startedCount = true;
			braces++;
		}
		else if (tokens[it].type == Token::CLOSE_BRACKET) {
			startedCount = true;
			braces--;
		}
		if (!braces && startedCount)
			break;
		it++;
		distance++;
	}
	if (it == end)
		PERR_EXIT(1, "Error: Invalid block scope");
	return distance;
}

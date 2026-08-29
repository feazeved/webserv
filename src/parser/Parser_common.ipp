#pragma once
#include "Parser.hpp"

static inline
bool s_is_config_delimiter(char value) {
	return value == '{' || value == '}' || value == ';';
}

static inline
bool s_length_check(u32 length) {
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

// TODO: Implement minimum size, and file querying before not after
PARSER_INL
(bool) s_read_whole_file(const char *filePath, usize &fileOffset, usize &fileSize, usize padSize) {
	struct stat st;
	if (stat(filePath, &st) == -1 || st.st_size < 16 || st.st_size >= UINT32_MAX)
		PERR_RETURN(1, "Error: Invalid file");

	int fd = open(filePath, O_RDONLY);
	if (fd == -1)
		PERR_RETURN(1, "Error: Failed to open file");

	fileSize = (usize) st.st_size;
	fileOffset = Arena::alloc_b(fileSize + 1 + padSize);
	if (fileOffset == UINT32_MAX) {
		close(fd);
		PERR_RETURN(1, "Error: Out of memory");
	}

	u8* ptr = Arena::mptr(fileOffset);
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
	return 0;
}

PARSER_INL
(Parser::Directive) s_build_directive(const Array32<Token> &tokens, usize &cursor, usize end) {
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
	if (dir.args.alloc_a((u32)(cursor - argumentStart)) == true)
		std::exit(1);
	for (u32 index = 0; index < dir.args.count; index++)
		dir.args[index] = tokens[argumentStart + index].value;
	return dir;
}

PARSER_INL
(usize) s_find_scope_end(const Array32<Token> &tokens, usize begin, usize end) {
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

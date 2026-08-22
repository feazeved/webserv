#pragma once
#include "core.hpp"
#include "HTTP.hpp"
#include "StringView.hpp"

namespace HTTP {

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

static inline
usize s_strtol10(const char *str, usize length) {
	usize value = 0;
	usize digit = 0;
	const char *ostr = str;
	const char *end = str + length;

	while (true) {
		digit = (usize) g_asciiLut[(u8)*str];
		if (value >= ((SIZE_MAX - 9) / 10))	// 9 being fixed reduces branching
			return SIZE_MAX;
		if (digit > 9)
			break;
		str++;
		value = value * 10 + digit;
	}
	if (str == ostr || str != end)
		return SIZE_MAX;
	return value;
}

static inline
usize s_find_scope_end(const Array32<Token> &tokens, usize begin, usize end) {
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
		PERR_EXIT(1, "Error: Invalid block");
	return distance;
}

static inline
usize s_count_locations(const Array32<Token> &tokens, usize cursor, usize end) {
	usize locationCount = 0;
	usize locationSize;

	while (cursor < end) {
		if (tokens[cursor].value == "location") {
			usize distance = s_find_scope_end(tokens, cursor, end);
			locationSize = tokens[cursor + distance].value.offset - tokens[cursor].value.offset + 1;
			if (locationSize > MAX_LOCATION_BLOCK_SIZE)
				PERR_EXIT(1, "Error: Location block exceeds 64 KiB");
			locationCount++;
			cursor += distance + 1;
		}
		else {
			while (cursor < end && tokens[cursor].type != Token::SEMICOLON)
				cursor++;
			if (cursor < end)
				cursor++;
		}
	}
	return locationCount;
}

static inline
usize s_count_servers(const char *str, usize length) {
	const char *end = str + length;
	usize serverCount = 0;
	isize pdepth = 0;

	while (str < end) {
		while (IS_SPACE(*str))
			str++;
		if (MEMCMP_INLINE(str, "server") != 0) {
			if (*str == 0)
				return serverCount;
			return SIZE_MAX;
		}
		str += 6;
		while (IS_SPACE(*str))
			str++;
		pdepth = (*str == '{') ? 1 : -1;
		str++;
		for (; str < end && pdepth > 0; str++) {
			for (; *str != '}'; str++)
				pdepth += *str == '{';
			if (str < end)
				pdepth--;
		}
		if (pdepth != 0)
			return SIZE_MAX;
		serverCount++;
	}
	return serverCount;
}

}

#pragma once

#include "core.hpp"

static inline
u64 s_epoll_server_key(usize index) {
	return ((u64) index << 1) | 1;
}

static inline
u64 s_epoll_connection_key(usize index) {
	return (u64) index << 1;
}

static inline
bool s_epoll_key_is_server(u64 key) {
	return (key & 1) != 0;
}

static inline
usize s_epoll_key_index(u64 key) {
	return (usize)(key >> 1);
}

static inline
usize s_strtol10(const char *str, usize length) {
	usize value = 0;
	usize digit = 0;
	const char *end = str + length;

	while (true) {
		digit = (usize) g_asciiLut[(u8)*str];
		if (value >= ((SIZE_MAX - 9) / 10))
			return SIZE_MAX;
		if (digit > 9)
			break;
		str++;
		value = value * 10 + digit;
	}
	if (str != end)
		return SIZE_MAX;
	return value;
}

static inline
bool s_is_config_delimiter(char value) {
	return value == '{' || value == '}' || value == ';';
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
			while (*str > 32 && !s_is_config_delimiter(*str))
				str++;
			if (str == word)
				str++;
		}
	}
	return tokenCount;
}

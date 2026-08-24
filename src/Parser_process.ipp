#pragma once

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "core.hpp"
#include "Parser.hpp"

namespace HTTP {

static inline
bool s_next_cgi_word(char *&cursor, char *end, StringView &word) {
	while (cursor != end && ((u8)*cursor <= 32 || s_is_config_delimiter(*cursor)))
		cursor++;
	if (cursor == end)
		return false;

	word.ptr = cursor;
	while (cursor != end && (u8)*cursor > 32 && !s_is_config_delimiter(*cursor))
		cursor++;
	word.length = (usize)(cursor - (const char*)word.ptr);
	return true;
}

/*
	Each normalized CGI entry contains two u16 lengths followed by the
	extension bytes and interpreter bytes.
*/
PARSER_INL
(void) process_cgi_block(VirtualServer &server) {
	u8 buffer[MAX_LOCATION_BLOCK_SIZE];

	for (u32 i = 0; i < server.locations.count; i++) {
		StringView32 &block = server.locations[i].cgiBlock;
		if (block.length == 0)
			continue;

		char *cursor = block.c_str_mut();
		char *const end = cursor + block.length;
		usize packSize = 0;
		StringView extension, interpreter;

		while (s_next_cgi_word(cursor, end, extension)) {
			s_next_cgi_word(cursor, end, interpreter);
			s_next_cgi_word(cursor, end, interpreter);

			const u16 lengths[2] = {(u16)extension.length, (u16)interpreter.length};

			MEMCPY_INLINE(buffer + packSize, lengths, sizeof(lengths));
			packSize += sizeof(lengths);
			MEMCPY(buffer + packSize, extension.ptr, extension.length);
			packSize += extension.length;
			MEMCPY(buffer + packSize, interpreter.ptr, interpreter.length);
			packSize += interpreter.length;
		}

		MEMCPY(Arena::get_ptr(block.offset), buffer, packSize);
		block.length = (u32)packSize;
	}
}
}

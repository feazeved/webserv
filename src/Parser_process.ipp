#pragma once

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "core.hpp"
#include "Parser.hpp"

namespace HTTP {

static inline
bool s_next_cgi_word(const char *&cursor, const char *end, StringView &ext) {
	while (cursor != end && ((u8)*cursor <= 32 || s_is_config_delimiter(*cursor)))
		cursor++;
	if (cursor == end)
		return false;

	ext.ptr = (u8*) cursor;
	while (cursor != end && (u8)*cursor > 32
		&& !s_is_config_delimiter(*cursor))
		cursor++;
	ext.length = (usize)(cursor - (char*)ext.ptr);
	return true;
}

/*
	Each normalized CGI entry is stored in-place as two big-endian u16 lengths,
	followed by the extension bytes and interpreter bytes. cgiBlock.length marks
	the end of the packed entry sequence.
*/
PARSER_INL
(void) process_cgi_block(VirtualServer &server) {
	u8 buffer[MAX_LOCATION_BLOCK_SIZE];

	for (u32 i = 0;	i < server.locations.count;	i++) {
		StringView32 &block = server.locations[i].cgiBlock;

		if (block.length == 0)
			continue;

		const char *cursor = block.get();
		const char *const end = cursor + block.length;
		usize packSize = 0;
		StringView extension, interpreter;

		while (s_next_cgi_word(cursor, end, extension)) {
			s_next_cgi_word(cursor, end, interpreter);
			s_next_cgi_word(cursor, end, interpreter);

			buffer[packSize++] = (u8)(extension.length >> 8);
			buffer[packSize++] = (u8)extension.length;
			buffer[packSize++] = (u8)(interpreter.length >> 8);
			buffer[packSize++] = (u8)interpreter.length;

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

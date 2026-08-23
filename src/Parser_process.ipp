#pragma once

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "core.hpp"
#include "Parser.hpp"

namespace HTTP {

static inline
bool s_next_cgi_word(const char *&cursor, const char *end,
	const char *&word, usize &length) {
	while (cursor != end && ((u8)*cursor <= 32 || s_is_config_delimiter(*cursor)))
		cursor++;
	if (cursor == end)
		return false;

	word = cursor;
	while (cursor != end && (u8)*cursor > 32
		&& !s_is_config_delimiter(*cursor))
		cursor++;
	length = (usize)(cursor - word);
	return true;
}

/*
	Each normalized CGI entry is stored in-place as two big-endian u16 lengths,
	followed by the extension bytes and interpreter bytes. cgiBlock.length marks
	the end of the packed entry sequence.
*/
PARSER_INL
(void) process_cgi_block(VirtualServer &server) {
	u8 packed[MAX_LOCATION_BLOCK_SIZE];

	for (u32 locationIndex = 0; locationIndex < server.locations.count; locationIndex++) {
		StringView &block = server.locations[locationIndex].cgiBlock;
		if (block.length == 0)
			continue;

		const char *cursor = block.get();
		const char *const end = cursor + block.length;
		usize packedSize = 0;
		const char *extension;
		const char *equals;
		const char *interpreter;
		usize extensionLength;
		usize equalsLength;
		usize interpreterLength;

		while (s_next_cgi_word(cursor, end, extension, extensionLength)) {
			if (!s_next_cgi_word(cursor, end, equals, equalsLength)
				|| !s_next_cgi_word(cursor, end, interpreter, interpreterLength)
				|| extensionLength < 2 || extension[0] != '.'
				|| equalsLength != 1 || equals[0] != '='
				|| extensionLength > UINT16_MAX
				|| interpreterLength > UINT16_MAX
				|| packedSize + 4 + extensionLength + interpreterLength > sizeof(packed))
				PERR_EXIT(true, "Error: Invalid CGI block encoding");

			packed[packedSize++] = (u8)(extensionLength >> 8);
			packed[packedSize++] = (u8)extensionLength;
			packed[packedSize++] = (u8)(interpreterLength >> 8);
			packed[packedSize++] = (u8)interpreterLength;
			MEMCPY(packed + packedSize, extension, extensionLength);
			packedSize += extensionLength;
			MEMCPY(packed + packedSize, interpreter, interpreterLength);
			packedSize += interpreterLength;
		}

		if (packedSize == 0)
			PERR_EXIT(true, "Error: Empty CGI block encoding");
		MEMCPY(Arena::data + block.offset, packed, packedSize);
		block.length = (u32)packedSize;
	}
}
}

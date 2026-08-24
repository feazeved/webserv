#pragma once

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "core.hpp"
#include "Parser.hpp"

namespace HTTP {

static inline
void s_build_error_page_path(char *out, const StringView32 &root, const StringView32 &path) {
	usize length = 0;
	if (root.length != 0) {
		MEMCPY(out, root.kptr(), root.length);
		length = root.length;
	}

	usize pathOffset = 0;
	if (root.length != 0 && path.length != 0) {
		const bool rootHasSlash = out[length - 1] == '/';
		const bool pathHasSlash = path.kptr()[0] == '/';
		if (rootHasSlash && pathHasSlash)
			pathOffset = 1;
		else if (!rootHasSlash && !pathHasSlash)
			out[length++] = '/';
	}

	const usize pathLength = path.length - pathOffset;
	MEMCPY(out + length, path.kptr() + pathOffset, pathLength);
	length += pathLength;
	out[length] = '\0';
}

PARSER_INL
(void) cache_error_pages(VirtualServer &server) {
	char pathBuffer[4 * MAX_PATH_SIZE];
	usize tmpSize, tmpOffset;
	StringView32 configuredPaths[Status::errorPageCount];

	for (usize index = 0; index < Status::errorPageCount; index++)
		configuredPaths[index] = server.errorPages[index];

	for (usize index = 0; index < Status::errorPageCount; index++) {
		StringView32 &page = server.errorPages[index];
		const StringView32 &path = configuredPaths[index];
		if (path.length == 0) {
			page = Status::s_error_str(index);
			continue;
		}

		usize duplicate = 0;
		for (; duplicate < index; duplicate++) {
			const StringView32 &previousPath = configuredPaths[duplicate];
			if (path.length == previousPath.length
				&& previousPath.length != 0
				&& MEMCMP(path.kptr(), previousPath.kptr(), path.length) == 0)
				break;
		}
		if (duplicate != index) {
			page = server.errorPages[duplicate];
			continue;
		}
		s_build_error_page_path(pathBuffer, server.serverRoot, path);
		if (s_read_whole_file(pathBuffer, tmpSize, tmpOffset, 0))
			_exit(1);
		page.offset = tmpOffset;
		page.length = tmpSize;
	}
}

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

		char *cursor = block.mptr();
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

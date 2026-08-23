#pragma once

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "core.hpp"
#include "StringView.hpp"
#include "Parser.hpp"

namespace HTTP {

static inline
void s_build_error_page_path(char *out, const StringView32 &root, const StringView32 &path) {
	usize length = 0;
	if (root.length != 0) {
		MEMCPY(out, root.get(), root.length);
		length = root.length;
	}

	usize pathOffset = 0;
	if (root.length != 0 && path.length != 0) {
		const bool rootHasSlash = out[length - 1] == '/';
		const bool pathHasSlash = path.get()[0] == '/';
		if (rootHasSlash && pathHasSlash)
			pathOffset = 1;
		else if (!rootHasSlash && !pathHasSlash)
			out[length++] = '/';
	}

	const usize pathLength = path.length - pathOffset;
	MEMCPY(out + length, path.get() + pathOffset, pathLength);
	length += pathLength;
	out[length] = '\0';
}

static inline
StringView32& s_error_page_at(VirtualServer &server, usize index) {
	if (index < s_client_error_count)
		return server.clientErrors[index];
	return server.serverErrors[index - s_client_error_count];
}

/*
	TODO: These need to access the flattened string definition of the errors in poolB
	Probably the same way Status does it, by keeping indexes. Might be a good idea to move
	the access error functions to the Status class
*/

PARSER_INL
(void) cache_error_pages(VirtualServer &server) {
	char pathBuffer[16384];
	usize tmpSize, tmpOffset;
	StringView configuredPaths[s_error_page_count];

	for (usize index = 0; index < s_error_page_count; index++)
		configuredPaths[index] = s_error_page_at(server, index);

	for (usize index = 0; index < s_error_page_count; index++) {
		StringView32 &page = s_error_page_at(server, index);
		const StringView32 &path = configuredPaths[index];
		if (path.length == 0) {
			page = s_default_error_pages[index];
			continue;
		}

		usize duplicate = 0;
		for (; duplicate < index; duplicate++) {
			const StringView32 &previousPath = configuredPaths[duplicate];
			if (path.length == previousPath.length
				&& previousPath.length != 0
				&& MEMCMP(path.get(), previousPath.get(), path.length) == 0)
				break;
		}
		if (duplicate != index) {
			page = s_error_page_at(server, duplicate);
			continue;
		}
		s_build_error_page_path(pathBuffer, server.serverRoot, path);
		if (s_read_whole_file(pathBuffer, tmpSize, tmpOffset, 0))
			_exit(1);
		page.offset = tmpOffset;
		page.length = tmpSize;
	}
}
}

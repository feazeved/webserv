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
		MEMCPY(out, root.c_str(), root.length);
		length = root.length;
	}

	usize pathOffset = 0;
	if (root.length != 0 && path.length != 0) {
		const bool rootHasSlash = out[length - 1] == '/';
		const bool pathHasSlash = path.c_str()[0] == '/';
		if (rootHasSlash && pathHasSlash)
			pathOffset = 1;
		else if (!rootHasSlash && !pathHasSlash)
			out[length++] = '/';
	}

	const usize pathLength = path.length - pathOffset;
	MEMCPY(out + length, path.c_str() + pathOffset, pathLength);
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
			page = Status::default_error_page(Status::error_code(index));
			continue;
		}

		usize duplicate = 0;
		for (; duplicate < index; duplicate++) {
			const StringView32 &previousPath = configuredPaths[duplicate];
			if (path.length == previousPath.length
				&& previousPath.length != 0
				&& MEMCMP(path.c_str(), previousPath.c_str(), path.length) == 0)
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
}

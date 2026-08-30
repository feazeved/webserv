#pragma once
#include "Parser.hpp"

static inline
void s_build_error_page_path(char *out, const Span &root, const Span &path) {
	usize length = 0;
	if (root.length != 0) {
		MEMCPY(out, root.ptr, root.length);
		length = root.length;
	}

	usize pathOffset = 0;
	if (root.length != 0 && path.length != 0) {
		const bool rootHasSlash = out[length - 1] == '/';
		const bool pathHasSlash = path.ptr[0] == '/';
		if (rootHasSlash && pathHasSlash)
			pathOffset = 1;
		else if (!rootHasSlash && !pathHasSlash)
			out[length++] = '/';
	}

	const usize pathLength = path.length - pathOffset;
	MEMCPY(out + length, path.ptr + pathOffset, pathLength);
	length += pathLength;
	out[length] = '\0';
}

PARSER_INL
(void) cache_error_pages(VirtualServer &server) {
	char pathBuffer[4 * MAX_PATH_SIZE];
	Span configuredPaths[Status::errorPageCount];

	for (usize index = 0; index < Status::errorPageCount; index++)
		configuredPaths[index] = server.errorPages[index];

	for (usize index = 0; index < Status::errorPageCount; index++) {
		Span &page = server.errorPages[index];
		const Span &path = configuredPaths[index];
		if (path.length == 0) {
			page = Status::s_error_str(index);
			continue;
		}

		usize duplicate = 0;
		for (; duplicate < index; duplicate++) {
			const Span &previousPath = configuredPaths[duplicate];
			if (path.length == previousPath.length
				&& previousPath.length != 0
				&& MEMCMP(path.ptr, previousPath.ptr, path.length) == 0)
				break;
		}
		if (duplicate != index) {
			page = server.errorPages[duplicate];
			continue;
		}
		s_build_error_page_path(pathBuffer, server.serverRoot, path);
		if (s_read_whole_file(beta, pathBuffer, page, 0, 0))
			std::exit(1);
	}
}

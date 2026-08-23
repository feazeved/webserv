#pragma once

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "core.hpp"
#include "StringView.hpp"
#include "Parser.hpp"

namespace HTTP {

static const char *const s_client_error_reasons[32] = {
	"Bad Request",
	"Unauthorized",
	"Payment Required",
	"Forbidden",
	"Not Found",
	"Method Not Allowed",
	"Not Acceptable",
	"Proxy Authentication Required",
	"Request Timeout",
	"Conflict",
	"Gone",
	"Length Required",
	"Precondition Failed",
	"Content Too Large",
	"URI Too Long",
	"Unsupported Media Type",
	"Range Not Satisfiable",
	"Expectation Failed",
	"(Unused)",
	"Client Error",
	"Client Error",
	"Misdirected Request",
	"Unprocessable Content",
	"Locked",
	"Failed Dependency",
	"Too Early",
	"Upgrade Required",
	"Client Error",
	"Precondition Required",
	"Too Many Requests",
	"Client Error",
	"Request Header Fields Too Large"
};

static const char *const s_server_error_reasons[12] = {
	"Internal Server Error",
	"Not Implemented",
	"Bad Gateway",
	"Service Unavailable",
	"Gateway Timeout",
	"HTTP Version Not Supported",
	"Variant Also Negotiates",
	"Insufficient Storage",
	"Loop Detected",
	"Server Error",
	"Not Extended",
	"Network Authentication Required"
};

static const usize s_client_error_count = ARRAY_SIZE(s_client_error_reasons);
static const usize s_server_error_count = ARRAY_SIZE(s_server_error_reasons);
static const usize s_error_page_count = s_client_error_count + s_server_error_count;
static StringView s_default_error_pages[s_error_page_count];
static usize s_default_error_pages_epoch = SIZE_MAX;

static inline
void s_copy(char *&out, const char *source, usize length) {
	MEMCPY(out, source, length);
	out += length;
}

static inline
bool s_make_default_error_page(usize index, StringView &out) {
	static const char prefix[] =
		"<!doctype html><html lang=en><meta charset=utf-8>"
		"<meta name=viewport content=\"width=device-width\"><title>";
	static const char middle[] =
		"</title><style>body{margin:0;min-height:100vh;display:grid;"
		"place-items:center;font:16px system-ui,sans-serif;background:#f6f7f9;"
		"color:#1f2937}main{text-align:center;padding:2rem}h1{font-size:5rem;"
		"line-height:1;margin:0}p{margin:.75rem 0 0;color:#667085}</style>"
		"<main><h1>";
	static const char beforeReason[] = "</h1><p>";
	static const char suffix[] = "</p></main></html>";

	const bool clientError = index < s_client_error_count;
	const usize code = clientError ? 400 + index : 500 + index - s_client_error_count;
	const char *reason = clientError ? s_client_error_reasons[index] : s_server_error_reasons[index - s_client_error_count];
	const usize reasonLength = STRLEN(reason);
	const usize pageSize = sizeof(prefix) - 1 + 4 + reasonLength
		+ sizeof(middle) - 1 + 3 + sizeof(beforeReason) - 1
		+ reasonLength + sizeof(suffix) - 1;
	const u32 offset = Arena::alloc_index(pageSize + 1);
	if (offset == UINT32_MAX)
		return true;

	char codeString[3];
	codeString[0] = (char)('0' + code / 100);
	codeString[1] = (char)('0' + code / 10 % 10);
	codeString[2] = (char)('0' + code % 10);

	char *ptr = (char*)Arena::data + offset;
	s_copy(ptr, prefix, sizeof(prefix) - 1);
	s_copy(ptr, codeString, sizeof(codeString));
	*ptr++ = ' ';
	s_copy(ptr, reason, reasonLength);
	s_copy(ptr, middle, sizeof(middle) - 1);
	s_copy(ptr, codeString, sizeof(codeString));
	s_copy(ptr, beforeReason, sizeof(beforeReason) - 1);
	s_copy(ptr, reason, reasonLength);
	s_copy(ptr, suffix, sizeof(suffix) - 1);
	*ptr = '\0';
	out = StringView((u32)pageSize, offset);
	return false;
}

PARSER_INL
(bool) cache_default_error_pages() {
	const usize arenaMark = Arena::size;
	for (usize index = 0; index < s_error_page_count; index++) {
		if (s_make_default_error_page(index, s_default_error_pages[index])) {
			Arena::size = arenaMark;
			return true;
		}
	}
	s_default_error_pages_epoch = Arena::epoch;
	return false;
}

static inline
void s_build_error_page_path(char *out, const StringView &root, const StringView &path) {
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
StringView& s_error_page_at(VirtualServer &server, usize index) {
	if (index < s_client_error_count)
		return server.clientErrors[index];
	return server.serverErrors[index - s_client_error_count];
}

PARSER_INL
(void) cache_error_pages(VirtualServer &server) {
	char pathBuffer[16384];
	usize tmpSize, tmpOffset;
	StringView configuredPaths[s_error_page_count];

	for (usize index = 0; index < s_error_page_count; index++)
		configuredPaths[index] = s_error_page_at(server, index);

	for (usize index = 0; index < s_error_page_count; index++) {
		StringView &page = s_error_page_at(server, index);
		const StringView &path = configuredPaths[index];
		if (path.length == 0) {
			page = s_default_error_pages[index];
			continue;
		}

		usize duplicate = 0;
		for (; duplicate < index; duplicate++) {
			const StringView &previousPath = configuredPaths[duplicate];
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

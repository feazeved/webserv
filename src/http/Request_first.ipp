#pragma once
#include "Request.hpp"

namespace HTTP {

REQUEST_INL
(isize) check_location(Cursor &src, ServerConfig* cfg) {
	bool found = false;
	std::vector<Location>::iterator it = cfg->locations.begin();

	const u8 *pathPtr = src.memStart + path.index;
	const usize pathLength = path.size;

	for(; it != cfg->locations.end(); it++)
	{
		if (MEMCMP(pathPtr, it->path.c_str(), it->path.size()) == 0 && pathLength == it->path.size())
		{
			found = true;
			break ;
		}
	}

	if (found)
		return !!(it->methods & (mode & 7));
	return false;
}

REQUEST_INL
(isize) parse_target(Cursor &src, ServerConfig* cfg) {

	u8 *const lineStart = src.linePtr;
	u8* &ptr = src.linePtr;
	u8 *end = src.lineEnd;

	query.index = 0;
	query.size = 0;
	path.index = 0;
	path.size = src.lineEnd - src.linePtr;
	while (ptr < end) {
		if (g_asciiLut[*ptr] > ASCII_RFC_SYMBOLS) {
			if (*ptr != '?')
				return -1;
			path.size = ptr - lineStart;
			ptr++;
			query.index = ptr - src.memStart;
			query.size = src.lineEnd - ptr;
			break;
		}
		if (*ptr == '%') {
			if (!(g_asciiLut[ptr[1]] <= ASCII_HEX && g_asciiLut[ptr[2]] <= ASCII_HEX))
				return -1;
			ptr += 2;
		}
		ptr++;
	}
	contentType = src.match_mime();	// TODO: 

	if(!check_location(src, cfg))
		return -1;
	return 0;
}

REQUEST_INL
(isize) parse_first_line(Cursor &src, ServerConfig* cfg) {
	const usize lineLength = src.lineEnd - src.linePtr;
	if (lineLength < 14 || lineLength >= 8192)
		return -1;	// ERROR: Bad request "GET / HTTP/1.1" shortest possible

	if (src.strcmp("GET"))
		mode |= Mode::GET;
	else if (src.strcmp("POST"))
		mode |= Mode::POST;
	else if (src.strcmp("DELETE"))
		mode |= Mode::DELETE;
	else
		return -1;	// ERROR: Invalid method

	src.lineEnd -= 9;
	if (MEMCMP(src.lineEnd, " HTTP/1.1", 9) != 0)
		return -1; // ERROR: Invalid version
	*src.lineEnd = 0;
	i32 rvalue = parse_target(src, cfg);	// TODO: meaningful return
	if (rvalue < 0)
		return rvalue;
	mode = Mode::PARSING;
	return 0;	// No problems (YET, return code for success only happens when finally executing the method)
}

// HTTP NAMESPACE END
}

#pragma once
#include "Request.hpp"

namespace HTTP {

static inline
isize s_get_mime_type(const char *dotPos, const char *end) {
	static const char *mimeStrings[] = {"\x09" "text/html", "\x09" "text/html", 
	"\x08" "text/css", "\x10" "application/json", "\x16" "application/javascript",
	"\x09" "image/png", "\x0A" "image/jpeg", "\x0A" "image/jpeg", 
	"\x09" "image/gif", "\x0A" "text/plain", "\x18" "application/octet-stream"};

	// isize matchId = match_mime_type(ptr, end);
	// cache.append("Content-Type: ");
	// cache.appendInline(mimeStrings[matchId] + 1, mimeStrings[matchId][0]);

}

REQUEST_INL
(isize) check_location(Buffer<bufferSize> &src, ServerConfig* cfg) {
	bool found = false;
	std::vector<Location>::iterator it = cfg->locations.begin();

	const char *pathPtr = src.data + path.index;
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
(isize) parse_target(Buffer<bufferSize> &src, ServerConfig* cfg) {
	const char *p = str;
	const char *questionMark = NULL;

	while (p < end) {
		if (*p <= 32)
			return -1;
		if (*p == '?') {
			if(!questionMark)
				questionMark = p;
			else
				return -1;
		}
		if (*p == '.' && p[1] == '.')
			return -1;
		if (*p == '%' && ((!IS_DIGIT(p[1]) || !IS_DIGIT(p[2]) || (p[1] == '0' && p[2] == '0'))))	// TODO: should be hex i was wrong
				return -1;
		p++;
	}

	path.index = str - (const char *)clientOutput.data;
	if (questionMark) {
		path.size = questionMark - str;
		query.index = (questionMark + 1) - (const char *)clientOutput.data;
		query.size = end - (questionMark + 1);
	}
	else
		path.size = end - str;

	if(!s_checkLocation(src.data + path.index, path.size, cfg))
		return -1;
	return 0;
}

REQUEST_INL
(isize) parse_first_line(Buffer<bufferSize> &src, ServerConfig* cfg) {
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

	if (MEMCMP(src.lineEnd - 9, "HTTP/1.1", 8) != 0)
		return -1; // ERROR: Invalid version

	i32 rvalue = parse_target(src, cfg);	// TODO: meaningful return
	if (rvalue < 0)
		return rvalue;
	return parse_header(src);	// No problems (YET, return code for success only happens when finally executing the method)
}

// HTTP NAMESPACE END
}

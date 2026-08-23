#pragma once
#include "Request.hpp"
#include "Array.hpp"

namespace HTTP {

/*
	TODO: request needs to validate ../ inputs as well
	Specifically, resolve them here so path.index points to
	a resolved file path
*/
static inline 
u16 s_check_location(u8 *ptr, usize length, VirtualServer* cfg, Request &request) {
	Array32<Location> &locations = cfg->locations;
	bool match = false;
	usize cmpLength = 1;
	while (cmpLength < length && ptr[cmpLength] != '/')
		cmpLength++;

	for (usize i = 0; i < locations.count; i++) {
		if (MEMCMP(ptr, locations[i].url.c_str(), cmpLength) == 0) {
			match = true;
			if ((locations[i].methods & (request.mode & 7)) != 0) {

				return (u16) i;
			}
		}
	}
	if (match == true)
		request.status = Status::i403;
	else
		request.status = Status::i404;
	return UINT16_MAX;
}

REQUEST_INL
(isize) parse_target(Cursor &src, VirtualServer* cfg) {
	u8 *const lineStart = src.readPtr;
	u8* &ptr = src.readPtr;
	u8 *end = src.lineEnd;

	if (*ptr != '/')
		return -1;
	query.index = 0;
	query.size = 0;
	path.index = 0;
	path.size = src.lineEnd - src.readPtr;
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
	locationIndex = s_check_location(lineStart, path.size, cfg, *this);
	if (locationIndex == UINT16_MAX)
		return -1;
	return 0;
}

// Returning 
REQUEST_INL
(isize) parse_first_line(Cursor &src, VirtualServer* cfg, usize lineLength) {
	if (lineLength < 14 || lineLength >= 8000)
		return -1;	// ERROR: Bad request "GET / HTTP/1.1" shortest possible

	if (src.strcmp("GET"))
		mode |= Mode::GET;
	else if (src.strcmp("POST"))
		mode |= Mode::POST;
	else if (src.strcmp("DELETE"))
		mode |= Mode::DELETE;
	else
		return -1;	// ERROR: Invalid method, TODO: set status

	src.lineEnd -= 9;
	if (MEMCMP(src.lineEnd, " HTTP/1.1", 9) != 0)
		return -1; // ERROR: Invalid version
	*src.lineEnd = 0;
	return parse_target(src, cfg);	// No problems (YET, return code for success only happens when finally executing the method)
}

// HTTP NAMESPACE END
}

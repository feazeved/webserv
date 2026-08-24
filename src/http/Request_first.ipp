#pragma once
#include "Request.hpp"
#include "Array.hpp"

namespace HTTP {

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
			if ((locations[i].methods & (request.options & 7)) != 0) {

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
(isize) parse_target(HTTP_Buffer &src, VirtualServer* cfg) {
	u8 *const lineStart = src.readPtr;
	u8* &ptr = src.readPtr;
	u8 *end = src.scanPtr;

	if (*ptr != '/')
		return -1;
	query.index = 0;
	query.size = 0;
	path.index = 0;
	path.size = src.scanPtr - src.readPtr;
	while (ptr < end) {
		if (g_asciiLut[*ptr] > ASCII_RFC_SYMBOLS) {
			if (*ptr != '?')
				return -1;
			path.size = ptr - lineStart;
			ptr++;
			query.index = ptr - src.data;
			query.size = src.scanPtr - ptr;
			break;
		}
		if (*ptr == '%') {
			if (!(g_asciiLut[ptr[1]] <= ASCII_HEX && g_asciiLut[ptr[2]] <= ASCII_HEX))
				return -1;
			ptr += 2;
		}
		ptr++;
	}
	contentType = src.match_mime();	// TODO: Finish
	locationIndex = s_check_location(lineStart, path.size, cfg, *this);
	if (locationIndex == UINT16_MAX)
		return -1;
	return 0;
}

REQUEST_INL
(isize) parse_first_line(HTTP_Buffer &src, VirtualServer* cfg, usize lineLength) {
	if (lineLength < 14 || lineLength >= 8000) {
		status = lineLength < 14 ? Status::i400 : Status::i431;
		return -1;	// ERROR: Bad request "GET / HTTP/1.1" shortest possible
	}
	u8 *const lineEnd = src.readPtr + lineLength;

	if (src.strcmp("GET "))
		options |= Options::GET;
	else if (src.strcmp("POST "))
		options |= Options::POST;
	else if (src.strcmp("DELETE "))
		options |= Options::DELETE;
	else {
		status = Status::i501;
		return -1;
	}

	if (MEMCMP(lineEnd - 9, " HTTP/1.1", 9) != 0) {
		status = Status::i505;
		return -1;
	}
	*(lineEnd - 9) = 0;
	const isize result = parse_target(src, cfg);
	if (result < 0 && !status.is_set())
		status = Status::i400;
	src.readPtr = src.scanPtr;	// TODO: add skip spaces
	return result;
}

}

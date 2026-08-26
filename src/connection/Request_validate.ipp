#pragma once
#include "Request.hpp"

static inline 
u16 s_check_location(u8 *ptr, usize length, VirtualServer* cfg, Request &request) {
	Array32<Location> &locations = cfg->locations;
	bool match = false;
	usize cmpLength = 1;
	while (cmpLength < length && ptr[cmpLength] != '/')
		cmpLength++;

	for (usize i = 0; i < locations.count; i++) {
		if (MEMCMP(ptr, locations[i].url.kptr(), cmpLength) == 0) {
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

static inline
Span16 s_check_cgi(Location *loc, Span refExt) {
	const char *basePtr = loc->cgiBlock.kptr();
	usize blockLength = loc->cgiBlock.length;
	u16 lengths[2];
	Span16 result;
	result.index = UINT16_MAX;
	usize i = 0;
	while (i < blockLength) {
		MEMCPY_INLINE(lengths, basePtr + i, sizeof(lengths));
		const char *ext = basePtr + i + sizeof(lengths);
		if (refExt.length == lengths[0] && MEMCMP(ext, refExt.ptr, refExt.length) == 0) {
			result.index = (u16)(i + lengths[0]);
			result.length = (u16)(lengths[1]);
			return result;
		}
		i += lengths[0] + lengths[1] + 1;	// +1 here right?
	}
	return result;
}

REQUEST_INL
(isize) validate_target(HTTP_Buffer &src, VirtualServer* cfg) {
	u8 *const lineStart = src.readPtr;

	if (src.check_target(path, query) == -1)
		return -1;
	locationIndex = s_check_location(lineStart, path.length, cfg, *this);
	if (locationIndex == UINT16_MAX)
		return -1;
	Span pathStr (path.index + (char*)src.data, path.length);
	interpreter = s_check_cgi(&cfg->locations[locationIndex], pathStr);
	if (interpreter.index == UINT16_MAX)
		contentType = src.match_mime();	// TODO: Is CGI a mime or octet stream?
	else
		options |= Options::CGI;
	return 0;
}

REQUEST_INL
(Mode::e_http_mode) validate_header(HTTP_Buffer &src, VirtualServer* cfg) {
	(void)src;
	const bool isBodyMethod = options & (Options::POST | Options::CGI);
	const bool encodingSet = options & (Options::CHUNKED_LENGTH | Options::FIXED_LENGTH);

	if (status.is_set())
		return Mode::CLOSE;	// An error caused early interruption

	if ((options & 0xF) == 0) {
		status = Status::i400;
		return Mode::CLOSE;	// TODO: Method not set, should be impossible. Remove in future
	}

	if ((options & Options::HOST) == 0) {
		status = Status::i400;
		return Mode::CLOSE;	// Host not set
	}

	if (isBodyMethod && !encodingSet) {
		status = Status::i411;
		return Mode::CLOSE;	// Transfer encoding not set
	}

	if (!isBodyMethod && encodingSet) {
		status = Status::i400;
		return Mode::CLOSE;	// Encoding set for non-body methods
	}

	if (options & Options::CHUNKED_LENGTH)
		bodySize = cfg->maxBodySize;

	return (Mode::e_http_mode) (options & 0x0F);
}

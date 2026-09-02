#pragma once
#include "Connection.hpp"

CONNECTION_INL
(bool) match_location() {
	ArrayView<Location> &locations = cfg->locations;
	usize cmpLength = *req.target.ptr == '/';
	while (cmpLength < req.target.size && req.target.ptr[cmpLength] != '/')
		cmpLength++;
	usize matchLength = 0;

	for (usize i = 0; i < locations.count; i++) {
		Span srcUri = locations[i].get_uri();
		if (cmpLength != srcUri.size || matchLength > srcUri.size)
			continue;
		if (MEMCMP(req.target.ptr, srcUri.ptr, cmpLength) == 0) {
			if (srcUri.size > matchLength) {
				matchLength = srcUri.size;
				req.location = &locations[i];
			}
		}
	}
	if (req.location == NULL) {
		status = Status::i404;
		return true;
	}
	if ((req.location->methods & (options & 7)))
		return false;
	status = Status::i405;
	return true;
}

CONNECTION_INL
(Span) check_cgi() {
	char *cgiEnd = req.cgi.end();
	u16 lengths[2];
	Span result = {};

	req.targetExt.ptr = req.target.ptr + req.target.size;
	req.targetName.ptr[-1] = '.';
	while (req.targetExt[-1] != '.')
		req.targetExt.ptr--;
	req.targetName.ptr[-1] = '/';

	while (req.cgi.ptr < cgiEnd) {
		MEMCPY_INLINE(lengths, req.cgi.ptr, sizeof(lengths));
		const char *ext = req.cgi.ptr + sizeof(lengths);
		if (req.targetExt.size == lengths[0] && MEMCMP(ext, req.targetExt.ptr, req.targetExt.size) == 0) {
			result.ptr = req.cgi.ptr + sizeof(lengths) + lengths[0];
			result.size = (u16)(lengths[1]);
			options |= Options::CGI;
			return result;
		}
		req.cgi.ptr += sizeof(lengths) + lengths[0] + lengths[1];
	}
	return result;
}

static inline
bool s_validate_path(char* str, char* end) {
	while (str < end) {
		if (g_asciiLut[(u8)*str] > ASCII_RFC_SYMBOLS)
			return true;
		if (*str == '%') {
			if (g_asciiLut[(u8)str[1]] > ASCII_HEX)
				return true;
			if (g_asciiLut[(u8)str[2]] > ASCII_HEX)
				return true;
			str += 2;
		}
		str++;
	}
	return false;
}

CONNECTION_INL
(isize) validate_target(char* str, char* end) {
	const usize targetLength = (usize)(end - str);
	// const char* lineStart = str;

	if (*str != '/')	// /images/cats/meow.jpg?FILTER=yes,ORDER=ascending\0
		return -1;
	char* queryPtr = (char*) MEMCHR(str, '?', targetLength);
	char* targetEnd = queryPtr == NULL ? end : (queryPtr + 1);
	req.target = Span::create(str, (usize)(targetEnd - str));		// /images/cats/meow.jpg
	req.query = Span::create(queryPtr, (usize)(end - targetEnd));	// FILTER=yes,ORDER=ascending\0
																	//		<---V
	req.targetName = Span::create(targetEnd, 0);					// /meow.jpg?FILTER=yes,ORDER=ascending\0
	while (req.targetName.ptr[-1] != '/')							// meow.jpg
		req.targetName.ptr--;
	req.targetName.size = (usize)(targetEnd - req.targetName.ptr);

	if (s_validate_path(req.target, targetEnd))
		return -1;
	if (match_location())											// /images
		return -1;

	req.uri = req.location->get_uri();
	req.cgi = req.location->get_cgi_block();
	*targetEnd = '\0';
	req.relativeTarget.ptr = req.target.ptr + req.uri.size;			// /images/cats/meow.jpg
	req.relativeTarget.size += req.target.size - req.uri.size;		// /cats/meow.jpg
	req.interpreter = check_cgi();
	return 0;
}

CONNECTION_INL
(isize) parse_first_line(usize lineLength) {
	if (lineLength < 14 || lineLength >= 8000) {
		status = lineLength < 14 ? Status::i400 : Status::i431;
		return -1;	// ERROR: Bad request "GET / HTTP/1.1" shortest possible
	}

	const usize readPosEnd = recvBuffer.readPos + lineLength - 9;
	char* targetEnd = recvBuffer.rptr() + lineLength - 9;
	if (recvBuffer.strcmp("GET "))
		options |= Options::GET;
	else if (recvBuffer.strcmp("POST "))
		options |= Options::POST;
	else if (recvBuffer.strcmp("DELETE "))
		options |= Options::DELETE;
	else {
		status = Status::i501;
		return -1;
	}
	char* targetStart = recvBuffer.rptr();
	recvBuffer.readPos = readPosEnd;
	if (recvBuffer.strcmp(" HTTP/1.1\r\n")) {
		status = Status::i505;
		return -1;
	}
	const isize result = validate_target(targetStart, targetEnd);
	if (result < 0 && !status.is_set())
		status = Status::i400;
	recvBuffer.readPos = recvBuffer.scanPos;
	return result;
}

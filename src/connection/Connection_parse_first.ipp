#pragma once
#include "Connection.hpp"

CONNECTION_INL
(Location*) match_location() {
	ArrayView<Location> &locations = cfg->locations;
	Location* loc = NULL;
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
				loc = &locations[i];
			}
		}
	}
	if (loc != NULL && !(loc->methods & (options & 7))) {
		status = Status::i405;
		return NULL;
	}
	else
		status = Status::i404;
	return loc;
}

static inline
Span s_check_cgi(Location *loc, Span refExt) {
	Span cgi = loc->get_cgi_block();
	char *cgiEnd = cgi.end();
	u16 lengths[2];
	Span result = {};

	while (cgi.ptr < cgiEnd) {
		MEMCPY_INLINE(lengths, cgi.ptr, sizeof(lengths));
		const char *ext = cgi.ptr + sizeof(lengths);
		if (refExt.size == lengths[0] && MEMCMP(ext, refExt.ptr, refExt.size) == 0) {
			result.ptr = cgi.ptr + sizeof(lengths) + lengths[0];
			result.size = (u16)(lengths[1]);
			return result;
		}
		cgi.ptr += sizeof(lengths) + lengths[0] + lengths[1];
	}
	return result;
}

static inline
Span s_target_extension(const Span &target) {
	for (usize index = target.size; index > 0; index--) {
		if (target.ptr[index - 1] == '/')
			break;
		if (target.ptr[index - 1] == '.') {
			Span result = {target.ptr + index - 1, target.size - index + 1};
			return result;
		}
	}
	Span result = {};
	return result;
}

CONNECTION_INL
(isize) validate_target(usize lineEnd) {
	if (recvBuffer.check_target(req.target, req.query, lineEnd) == -1)
		return -1;
	req.location = match_location();
	if (req.location == NULL)
		return -1;
	const Span extension = s_target_extension(req.target);
	req.interpreter = s_check_cgi(req.location, extension);
	if (req.interpreter.ptr == NULL) {
		const usize savedReadPos = recvBuffer.readPos;
		const usize savedScanPos = recvBuffer.scanPos;
		recvBuffer.readPos = (usize)(req.target.ptr - (char*)recvBuffer.data);
		recvBuffer.scanPos = recvBuffer.readPos + req.target.size;
		contentType = (u8)recvBuffer.match_mime();
		recvBuffer.readPos = savedReadPos;
		recvBuffer.scanPos = savedScanPos;
	}
	else
		options |= Options::CGI;
	return 0;
}

CONNECTION_INL
(isize) parse_first_line(usize lineLength) {
	if (lineLength < 14 || lineLength >= 8000) {
		status = lineLength < 14 ? Status::i400 : Status::i431;
		return -1;	// ERROR: Bad request "GET / HTTP/1.1" shortest possible
	}
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

	if (targetEnd <= recvBuffer.rptr()) {
		status = Status::i400;
		return -1;
	}
	if (MEMCMP(targetEnd, " HTTP/1.1", 9) != 0) {
		status = Status::i505;
		return -1;
	}
	*targetEnd = '\0';

	const isize result = validate_target((usize)(targetEnd - (char*)recvBuffer.data));
	if (result < 0 && !status.is_set())
		status = Status::i400;
	recvBuffer.readPos = recvBuffer.scanPos;
	return result;
}

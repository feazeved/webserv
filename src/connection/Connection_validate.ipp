#pragma once
#include "Connection.hpp"

CONNECTION_INL
(Location*) check_location() {
	Array<Location> &locations = cfg->locations;
	bool match = false;
	usize cmpLength = *req.target.ptr == '/';
	while (cmpLength < req.target.size && req.target.ptr[cmpLength] != '/')
		cmpLength++;

	for (usize i = 0; i < locations.count; i++) {
		Span srcUri = locations[i].get_uri();
		if (cmpLength != srcUri.size)
			continue;
		if (MEMCMP(req.target.ptr, srcUri.ptr, cmpLength) == 0) {
			match = true;
			if ((locations[i].methods & (options & 7)) != 0) {
				return &locations[i];
			}
		}
	}
	if (match == true)
		status = Status::i403;
	else
		status = Status::i404;
	return NULL;
}

static inline
Span s_check_cgi(Location *loc, Span refExt) {
	Span cgi = loc->get_cgi_block();
	char *cgiEnd = cgi.end();
	u16 lengths[2];
	Span result = {0, 0};

	while (cgi.ptr < cgiEnd) {
		MEMCPY_INLINE(lengths, cgi.ptr, sizeof(lengths));
		const char *ext = cgi.ptr + sizeof(lengths);
		if (refExt.size == lengths[0] && MEMCMP(ext, refExt.ptr, refExt.size) == 0) {
			result.ptr = cgi.ptr + lengths[0];
			result.size = (u16)(lengths[1]);
			return result;
		}
		cgi.ptr += lengths[0] + lengths[1] + 1;	// +1 here right?
	}
	return result;
}

CONNECTION_INL
(isize) validate_target() {
	if (recvBuffer.check_target(req.target, req.query) == -1)
		return -1;
	req.location = check_location();
	if (req.location == NULL)
		return -1;
	req.interpreter = s_check_cgi(req.location, req.target);
	if (req.interpreter.ptr == NULL)
		contentType = recvBuffer.match_mime();	// TODO: Is CGI a mime or octet stream?
	else
		options |= Options::CGI;
	return 0;
}

CONNECTION_INL
(isize) validate_header() {
	const bool isBodyMethod = options & (Options::POST | Options::CGI);
	const bool encodingSet = options & (Options::CHUNKED_LENGTH | Options::FIXED_LENGTH);

	if (status.is_set())
		return close_connection();	// An error caused early interruption

	if ((options & 0xF) == 0) {
		status = Status::i400;
		mode = Mode::CLOSE;
		return close_connection();	// TODO: Method not set, should be impossible. Remove in future
	}

	if ((options & Options::HOST) == 0) {
		status = Status::i400;
		mode = Mode::CLOSE;
		return close_connection();	// Host not set
	}

	if (isBodyMethod && !encodingSet) {
		status = Status::i411;
		mode = Mode::CLOSE;	// TODO: Should be useless to set it here
		return close_connection();	// Transfer encoding not set
	}

	if (!isBodyMethod && encodingSet) {
		status = Status::i400;
		mode = Mode::CLOSE;
		return close_connection();	// Encoding set for non-body methods
	}
	return setup();
}

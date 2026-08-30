#pragma once
#include "Connection.hpp"

CONNECTION_INL
(Location*) check_location() {
	Array<Location> &locations = cfg->locations;
	bool match = false;
	usize cmpLength = *req.path.ptr == '/';
	while (cmpLength < req.path.length && req.path.ptr[cmpLength] != '/')
		cmpLength++;

	for (usize i = 0; i < locations.count; i++) {
		Span srcUri = locations[i].get_uri();
		if (cmpLength != srcUri.length)
			continue;
		if (MEMCMP(req.path.ptr, srcUri.ptr, cmpLength) == 0) {
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
		if (refExt.length == lengths[0] && MEMCMP(ext, refExt.ptr, refExt.length) == 0) {
			result.ptr = cgi.ptr + lengths[0];
			result.length = (u16)(lengths[1]);
			return result;
		}
		cgi.ptr += lengths[0] + lengths[1] + 1;	// +1 here right?
	}
	return result;
}

CONNECTION_INL
(isize) validate_target() {
	if (recvBuffer.check_target(req.path, req.query) == -1)
		return -1;
	req.location = check_location();
	if (req.location == NULL)
		return -1;
	req.interpreter = s_check_cgi(req.location, req.path);
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
		return error_path();	// An error caused early interruption

	if ((options & 0xF) == 0) {
		status = Status::i400;
		mode = Mode::CLOSE;
		return error_path();	// TODO: Method not set, should be impossible. Remove in future
	}

	if ((options & Options::HOST) == 0) {
		status = Status::i400;
		mode = Mode::CLOSE;
		return error_path();	// Host not set
	}

	if (isBodyMethod && !encodingSet) {
		status = Status::i411;
		mode = Mode::CLOSE;	// TODO: Should be useless to set it here
		return error_path();	// Transfer encoding not set
	}

	if (!isBodyMethod && encodingSet) {
		status = Status::i400;
		mode = Mode::CLOSE;
		return error_path();	// Encoding set for non-body methods
	}

	if (options & Options::CHUNKED_LENGTH)
		bodySize = cfg->maxBodySize;

	mode = (Mode::e_http_mode)(options & 0x0F);
	if (mode == Mode::GET)
		return get_setup();
	if (mode == Mode::POST)
		return post_setup();
	if (mode == Mode::CGI)
		return cgi_setup();
	return del_setup();		// TODO: All setup calls should call dispatch again
}

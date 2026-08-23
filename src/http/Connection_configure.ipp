#pragma once
#include "Connection.hpp"

namespace HTTP {

// If there is any error here, then the connection always assumes the form of//////////////////////////////////////////
// stream the error until finished, then close connection
CONNECTION_INL
(isize) error_path() {
	if (readFd >= 0) {
		close(readFd);
		readFd = -1;
	}

	if (writeFd >= 0) {
		close(writeFd);
		writeFd = -1;
	}

	state = State::CLOSE;
	build_header();
	
	return -1;
}

CONNECTION_INL
(void) build_path(char* buffer, const char *ptr, usize length) {
	const StringView32& root = cfg->locations[request.locationIndex].root;

	MEMCPY(buffer, root.c_str(), root.length);
	buffer += root.length;
	MEMCPY(buffer, ptr, length);
	buffer[length] = 0;
}

CONNECTION_INL
(isize) configure() {
	if (request.options & Options::CHUNKED_LENGTH)
		request.bodySize = cfg->maxBodySize;

	if (request.mode & Mode::CGI)
		return cgi_first_run();
	else if (request.mode & Mode::POST)
		return post_first_run();
	else if (request.mode & Mode::DELETE)
		return del_first_run();
	else
		return get_first_run();
}
}

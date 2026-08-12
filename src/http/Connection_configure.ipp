#pragma once
#include "Connection.hpp"

namespace HTTP {

// If there is any error here, then the connection always assumes the form of
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
	
}

CONNECTION_INL
(isize) configure() {
	if (request.options & Options::CHUNKED_LENGTH)
		request.bodySize = cfg->maxBodySize;

	if (request.mode & Mode::CGI)
		return cgi_first_run();
	else if (request.mode & Mode::GET)
		return get_first_run();
	else if (request.mode & Mode::DELETE)
		return del_first_run();
	else
		return post_first_run();
}
}

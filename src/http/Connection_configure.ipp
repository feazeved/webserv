#pragma once
#include "Connection.hpp"

namespace HTTP {

CONNECTION_INL
(isize) error_path() {
		// Status should be set prior to entering error path
		// buildHeader();
		// Close the connection
		// Clean files
		// Reset state
}

CONNECTION_INL
(isize) configure() {

	request.bodySize = (request.bodySize == SIZE_MAX) ? SIZE_MAX : cfg->maxBodySize;
	if (request.mode & Mode::CGI)
		return cgi_first_run();
	else if (request.mode & (Mode::GET | Mode::DELETE))
		return get_first_run();
	else
		return post_first_run();
}
}

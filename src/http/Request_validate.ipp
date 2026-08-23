#pragma once
#include "Request.hpp"

namespace HTTP {
//

REQUEST_INL
(isize) validate_header(Cursor &src, VirtualServer* cfg) {
	const bool isBodyMethod = mode & (Mode::POST | Mode::CGI);
	const bool encodingSet = !(options & (Options::CHUNKED_LENGTH | Options::FIXED_LENGTH));

	if (status.is_set())
		return -1;	// An error caused early interruption

	if ((mode & 0xF) == 0)
		return -1;	// TODO: Method not set, should be impossible. Remove in future

	if ((mode & Options::HOST) == 0)
		return -1;	// Host not set

	if (isBodyMethod && !encodingSet)
		return -1;	// Transfer encoding not set

	if (!isBodyMethod && encodingSet)
		return -1;	// Encoding set for non-body methods

	return 1;
}

// HTTP NAMESPACE END
}

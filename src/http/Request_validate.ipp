#pragma once
#include "Request.hpp"

namespace HTTP {
//

REQUEST_INL
(Mode::e_http_mode) validate_header(HTTP_Buffer &src, VirtualServer* cfg) {
	(void)src;
	(void)cfg;
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

// HTTP NAMESPACE END
}

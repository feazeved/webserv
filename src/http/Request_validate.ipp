#pragma once
#include "Request.hpp"

namespace HTTP {
//

static inline
bool s_resolve_location(VirtualServer* cfg, const char* reqPath, usize reqPathLen, Location** outLocation, std::string& outRelative) {
	std::vector<Location>::iterator	it = cfg->locations.begin();
	for (; it != cfg->locations.end(); it++) {
		usize locLen = it->url.size();
		if (reqPathLen >= locLen && MEMCMP(reqPath, it->url.c_str(), locLen) == 0) {
			*outLocation = &(*it);
			outRelative.assign(reqPath + locLen, reqPathLen - locLen);
			return true;
		}
	}
	return false;
}

static inline
void s_join_path(const std::string& base, const std::string& relative, std::string& out) {
	out = base;
	if (!out.empty() && out[out.size() - 1] != '/' && (relative.empty() || relative[0] != '/'))
		out += '/';
	out += relative;
}

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

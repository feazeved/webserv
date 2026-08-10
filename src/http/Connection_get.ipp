#pragma once
#include "Connection.hpp"
#include "HTTP.hpp"
#include <cerrno>
#include <cstdio>
#include <fcntl.h>
#include <locale>
#include <vector>
#include <cstring>
#include <sys/stat.h>
#include <string>

namespace HTTP {

static inline
bool s_resolve_location(ServerConfig* cfg, const char* reqPath, usize reqPathLen, Location** outLocation, std::string& outRelative) {
	std::vector<Location>::iterator it = cfg->locations.begin();
	for (; it != cfg->locations.end(); it++) {
		usize	locLen = it->path.size();
		if (reqPathLen >= locLen && MEMCMP(reqPath, it->path.c_str(), locLen) == 0) {
			*outLocation = &(*it);
			outRelative.assign(reqPath + locLen, reqPathLen - locLen);
			return true;
		}
	}
	return false;
}

static inline
void s_join_path(const std::string &base, const std::string &relative, std::string &out) {
	out = base;
	if (!out.empty() && out[out.size() - 1] != '/' && (relative.empty() || relative[0] != '/'))
		out += '/';
	out += relative;
}

CONNECTION_INL
(isize) get_first_run() {
	const char*	reqPath = (const char*)clientOutput.data + request.path.index;
	Location*	location = NULL;
	std::string	relative;
	std::string	fullpath;

	// There is no location. Error 404
	if (!s_resolve_location(cfg, reqPath, request.path.size, &location, relative)) {
		request.status = Status::i404;
		return error_path();
	}
	s_join_path(location->root, relative, fullpath);

	struct stats st;
	if (stat(fullpath.c_str(), &st) == -1) {
		request.status = (errno == ENOENT) ? Status::i404 : (errno == EACCES ? Status::i403 : Status::i500);
		return error_path();
	}

	if (S_ISDIR(st.st_mode)) {
		if (location->index.empty() && location->autoindex == true) {
			// build autoindex
			request.status = Status::i403;
			return error_path();
		}
		std::string	indexPath = fullpath;
		if (!indexPath.empty() && indexPath[indexPath.size() - 1] != '/')
			indexPath += '/';
		indexPath += location->index;

		if (stat(indexPath.c_str(), &st) == -1) {
			request.status = (errno == ENOENT) ? Status::i404 : Status::i500;
			return error_path();
		}
		if (S_ISDIR(st.st_mode)) {	// Index is directory (maybe should be parsing error?)
			request.status = Status::i500;
			fullpath = indexPath;
			return error_path();
		}
	}

	i32	rawFd = open(fullpath.c_str(), O_RDONLY);
	if (rawFd == -1) {
		request.status = (errno == ENOENT) ? Status::i404 : (errno == EACCES ? Status::i403 : Status::i500);
		return error_path();
	}

	readFd = rawFd;
	writeFd = -1;
	request.status = Status::i200;

	return 0;
}

// Header will already be built in the configure function
CONNECTION_INL
(isize) get_method() {
	isize bytesRead = read_from_server();
	if (bytesRead < 0)
		return bytesRead;
	return bytesRead;
}

// namespace HTTP
}

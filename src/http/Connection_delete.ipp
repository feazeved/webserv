#pragma once
#include "Connection.hpp"

namespace HTTP {

CONNECTION_INL
(isize) del_first_run() {
	const char*	reqPath = (const char*)clientOutput.cursor.memStart + request.path.index;
	Location*	location = NULL;
	std::string	relative;
	std::string	fullPath;

	// There is no location. Error 404
	if (!s_resolve_location(cfg, reqPath, request.path.size, &location, relative)) {
		request.status = Status::i404;
		return -1;
	}
	s_join_path(location->root, relative, fullPath);

	struct stat	st;
	if (stat(fullPath.c_str(), &st) == -1) {
		request.status = (errno == ENOENT) ? Status::i404 : Status::i500;
		return -1;
	}
	if (S_ISDIR(st.st_mode)) {	// I think we shouldnt delete directories
		request.status = Status::i403;
		return -1;
	}
	if (unlink(fullPath.c_str()) == -1) {	// Maybe use std::remove bcs I dont see unlink in subject
		request.status = (errno == EACCES || errno == EPERM) ? Status::i403 : Status::i500;
		return -1;
	}

	readFd = -1;	// probably unnecessary
	writeFd = -1;

	request.status = Status::i204;
	build_header();

	// TODO: shouldnt we have this?
	// state = State::WRITING_TO_CLIENT;
	return 0;
}

CONNECTION_INL
(isize) del_method() {
	return 0;
}

// HTTP namespace
}

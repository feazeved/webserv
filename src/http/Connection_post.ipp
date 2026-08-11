#pragma once
#include "Connection.hpp"

namespace HTTP {

CONNECTION_INL
(isize) post_first_run() {
	const char*	reqPath = (const char*)clientOutput.cursor.data + request.path.index;
	Location*	location = NULL;
	std::string	relative;

	if (s_resolve_location(cfg, reqPath, request.path.size, &location, relative)) {
		request.status = Status::i404;
		return error_path();
	}

	if (relative.empty() || relative[relative.size() - 1] == '/') {
		request.status = Status::i400;
		return error_path();
	}

	const std::string&	uploadDir = location->upload_store.empty() ? location->root : location->upload_store;
	if (uploadDir.empty()) {
		request.status = Status::i500;
		return error_path();
	}

	std::string	destPath;
	s_join_path(uploadDir, relative, destPath);

	int	rawFd = open(destPath.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0644);
	if (rawFd == -1) {
		if (errno == EEXIST)
			request.status = Status::i409;
		else if (errno == EACCES)
			request.status = Status::i403;
		else
			request.status = Status::i500;
		return error_path();
	}

	writeFd = rawFd;
	readFd = -1;
	return 0;
}

CONNECTION_INL
(isize) post_method() {
	isize bytesWritten = write_to_server(numBytes);
	if (bytesWritten < 0)
		return bytesWritten;

	// Return path until the operation isnt complete
	if (!request.status.is_set() && writeFd == -1) {
		request.status = Status::i201;
		build_header();
	}
	return bytesWritten;
}
}

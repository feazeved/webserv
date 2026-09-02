#pragma once
#include "Connection.hpp"

CONNECTION_INL
(char*) append_target_path(Buffer64 &buffer) {
	const Span root = req.location->get_root();
	const Span uri = req.location->get_uri();

	char* fullPath = buffer.append(root);
	const Span suffix = {req.target.ptr + uri.size, req.target.size - uri.size};
	if (suffix.size != 0) {
		const bool rootHasSlash = root.size != 0 && root.ptr[root.size - 1] == '/';
		const bool suffixHasSlash = suffix.ptr[0] == '/';
		if (!rootHasSlash && !suffixHasSlash)
			buffer.append("/");
		buffer.append(suffix.ptr + (rootHasSlash && suffixHasSlash), suffix.size - (rootHasSlash && suffixHasSlash));
	}
	*buffer = 0;
	return fullPath;
}

static inline
Status::Code s_get_status() {
	Status::Code code;
	const int error = errno;

	if (error == ENOENT || error == ENOTDIR)
		code = Status::i404;
	else if (error == EACCES || error == EPERM || error == EROFS)
		code = Status::i403;
	else if (error == EEXIST || error == ENOTEMPTY || error == EBUSY)
		code = Status::i409;
	else if (error == ENAMETOOLONG)
		code = Status::i414;
	else if (error == ENOSPC || error == EDQUOT)
		code = Status::i507;
	else if (error == EMFILE || error == ENFILE || error == ENOMEM)
		code = Status::i503;
	else
		code = Status::i500;
	errno = 0;
	return code;
}

CONNECTION_INL
(isize) init(int fd, VirtualServer* serverConfig) {
	ASSERT(clientFd == -1, "Assigned a connection already in use");
	clientFd = fd;
	cfg = serverConfig;
	readFd = -1;
	writeFd = -1;
	processId = -1;
	ioState = EPOLLIN;
	options = 0;
	contentType = Mime::OCTET_STREAM;
	bodySize = 0;
	chunkSize = SIZE_MAX;
	mode = Mode::PARSE;
	recvBuffer.clear();
	req.clear();
	sendBuffer.clear();
	status.clear();
	startTime = Clock::time_elapsed();
	return 1;
}

CONNECTION_INL
(isize) end_connection() {
	clear();
	if (clientFd >= 0)
		close(clientFd);
	clientFd = -1;
	return -1;
}

CONNECTION_INL
(void) clear() {
	if (mode == Mode::AUTOINDEX && directory != NULL) {
		closedir(directory);
		directory = NULL;
		readFd = -1;
	}
	if (readFd >= 0)
		close(readFd);
	if (writeFd >= 0)
		close(writeFd);
	readFd = -1;
	writeFd = -1;
	processId = -1;
}

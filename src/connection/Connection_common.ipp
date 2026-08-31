#pragma once
#include "Connection.hpp"

static inline
isize s_get_status(Status &status) {
	const int error = errno;

	if (error == ENOENT || error == ENOTDIR)
		status = Status::i404;
	else if (error == EACCES || error == EPERM || error == EROFS)
		status = Status::i403;
	else if (error == EEXIST || error == ENOTEMPTY || error == EBUSY)
		status = Status::i409;
	else if (error == ENAMETOOLONG)
		status = Status::i414;
	else if (error == ENOSPC || error == EDQUOT)
		status = Status::i507;
	else if (error == EMFILE || error == ENFILE || error == ENOMEM)
		status = Status::i503;
	else
		status = Status::i500;
	errno = 0;
	return -1;
}

CONNECTION_INL
(isize) init(int fd, VirtualServer* serverConfig) {
	ASSERT(clientFd != -1, "Assigned a connection already in use");
	clientFd = fd;
	cfg = serverConfig;
	readFd = -1;
	writeFd = -1;
	processId = -1;
	mode = Mode::PARSE;
	recvBuffer.clear();
	sendBuffer.clear();
	startTime = Clock::time_elapsed();
	return 1;
}

CONNECTION_INL
(isize) close_connection(bool streamHeader) {
	if (readFd >= 0) {
		close(readFd);
		readFd = -1;
	}

	if (writeFd >= 0) {
		close(writeFd);
		writeFd = -1;
	}

	if (streamHeader) {
		build_header();
		options &= ~(u16)Options::KEEP_ALIVE;
		return 0;		// Keep the connection alive until header is flushed
	}

	return -1;
}

// TODO: Check if waiting is necessary, might be able to cull the child in Server
// Give a bitmap and an index, and it sets the index when a kill happens
CONNECTION_INL
(void) clear() {
	int status;

	if (mode == Mode::AUTOINDEX && directory != NULL) {
		closedir(directory);
		readFd = -1;
	}
	if (readFd >= 0)
		close(readFd);
	if (writeFd >= 0)
		close(writeFd);
	clientFd = -1;
	if (processId != -1) {
		kill(processId, SIGKILL);
		waitpid(processId, &status, WNOHANG);
		processId = -1;
	}
}

CONNECTION_INL
(bool) check_timeout(time_t curTime) {
	const time_t elapsed = curTime - startTime;

	if (elapsed > CONNECTION_TIMEOUT) {
		clear();
		return true;
	}
	return false;
}
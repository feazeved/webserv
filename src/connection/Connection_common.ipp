#pragma once
#include "Connection.hpp"

static inline
isize s_get_status(Status &status) {
	const int error = errno;

	if (error == ENOENT || error == ENOTDIR)
		status = Status::i404;
	if (error == EACCES || error == EPERM || error == EROFS)
		status = Status::i403;
	if (error == EEXIST || error == ENOTEMPTY || error == EBUSY)
		status = Status::i409;
	if (error == ENAMETOOLONG)
		status = Status::i414;
	if (error == ENOSPC || error == EDQUOT)
		status = Status::i507;
	if (error == EMFILE || error == ENFILE || error == ENOMEM)
		status = Status::i503;
	status = Status::i500;
	return -1;
}

static inline
void s_build_path(Buffer16 &buffer, Span &span, StringView32& root) {
	buffer.append(root.kptr(), root.length);
	buffer.append(span);
	buffer.append("\0");
}

// Check epoll, see if can write, if not, set to write and return 0
CONNECTION_INL
(isize) write_to_client(u32 events) {
	isize bytesWritten = recvBuffer.write(clientFd, ATOMIC_IOSIZE);
	if (bytesWritten <= 0)
		return close_connection();
	if (sendBuffer.size() != 0)
		return bytesWritten;
	if (mode == Mode::FLUSH) {
		mode = Mode::PARSE;
		return bytesWritten;
	}
	return close_connection();
}

/*
	Check epoll, see if can write, if not, set to write and return 0
	This is only called when information is needed, therefore if is 0 bytes
	are read, the connection should close. But maybe it's error_path instead
*/
CONNECTION_INL
(isize) read_from_client(u32 events) {
	isize bytesRead = recvBuffer.read(clientFd, ATOMIC_IOSIZE);
	if (bytesRead <= 0)
		return close_connection();
	return bytesRead;
}

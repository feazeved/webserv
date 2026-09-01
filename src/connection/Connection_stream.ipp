#pragma once
#include "Connection.hpp"

CONNECTION_INL
(isize) write_to_client(Epoll &epoll) {
	if (!epoll.is_writeable())
		return epoll.set_write(clientFd, 1);

	isize bytesWritten = sendBuffer.write(clientFd, ATOMIC_IOSIZE);
	return bytesWritten;
}

CONNECTION_INL
(isize) read_from_client(Epoll &epoll) {
	if (!epoll.is_readable())
		return epoll.set_read(clientFd, 1);

	isize bytesRead = recvBuffer.read(clientFd, ATOMIC_IOSIZE);
	return bytesRead;
}

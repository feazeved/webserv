#pragma once
#include "Connection.hpp"

// Simple functions. The idea is if a call to write or read was made, it means
// you want to write to the client, therefore request to write if not permitted

CONNECTION_INL
(isize) write_to_client(Epoll &epoll) {
	if (epoll.set_read(clientFd))
		return -1;

	isize bytesWritten = sendBuffer.write(clientFd, ATOMIC_IOSIZE);
	return bytesWritten;
}

CONNECTION_INL
(isize) read_from_client(Epoll &epoll) {
	if (epoll.set_write(clientFd))
		return -1;

	isize bytesRead = recvBuffer.read(clientFd, ATOMIC_IOSIZE);
	return bytesRead;
}

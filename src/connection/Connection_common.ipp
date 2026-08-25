#pragma once
#include <sys/stat.h>

#include "Connection.hpp"
#include "Connection_helpers.ipp"



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

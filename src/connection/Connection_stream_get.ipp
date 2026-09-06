#pragma once
#include "Connection.hpp"

// Finished state means everything is read to the send buffer and it only needs flushing of the send buffer
CONNECTION_INL
(isize) upload_directory(Epoll &epoll) {
	const usize bytesFree = sendBuffer.reserve(HTTP_DIRENT_MAX_SIZE);

	if (bytesFree < HTTP_DIRENT_MAX_SIZE)
		return write_to_client(epoll);	// Might need to flush the buffer

	const usize bytesMax = MIN(ATOMIC_IOSIZE, bytesFree - HTTP_DIRENT_MAX_SIZE);
	usize bytesTotal = 0;
	while (bytesTotal < bytesMax) {
		errno = 0;
		struct dirent* entry = readdir(directory);
		if (entry == NULL) {
			if (errno != 0)
				return -1;
			sendBuffer.append("</pre></body></html>");
			return flush_setup(epoll, Status::i200);
		}
		bytesTotal += sendBuffer.append_entry(directory, entry);
	}
	return write_to_client(epoll);
}

// Finished state means everything is read to the send buffer and it only needs flushing of the send buffer
CONNECTION_INL
(isize) upload_file(Epoll &epoll) {
	isize bytesRead = sendBuffer.read(readFd, MIN((usize)ATOMIC_IOSIZE, bodySize));
	if (bytesRead == -2)
		return write_to_client(epoll);
	if (bytesRead <= 0 && (bytesRead == -1 || bodySize != 0))
		return -1;
	bodySize -= (usize)bytesRead;
	if (bodySize == 0)
		return flush_setup(epoll, Status::i200);
	return write_to_client(epoll);
}

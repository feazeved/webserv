#pragma once
#include "Connection.hpp"

// Simple functions. The idea is if a call to write or read was made, it means
// you want to write to the client, therefore request to write if not permitted

CONNECTION_INL
(isize) flush(Epoll &epoll) {
	isize bytesWritten = write_to_client(epoll);
	if (sendBuffer.size() > 0)
		return bytesWritten;
	if (options & Options::KEEP_ALIVE)
		return parse_setup(epoll);
	return -1;	// Close the connection
}

CONNECTION_INL
(isize) write_to_client(Epoll &epoll) {
	if (!epoll.is_writeable())
		return 0;
	epoll.clr_write_flag();
	isize bytesWritten = sendBuffer.write(clientFd, ATOMIC_IOSIZE);
	return bytesWritten;
}

CONNECTION_INL
(isize) read_from_client(Epoll &epoll) {
	if (!epoll.is_readable())
		return 0;
	epoll.clr_read_flag();
	isize bytesRead = recvBuffer.read(clientFd, ATOMIC_IOSIZE);
	return bytesRead == 0 ? -1 : bytesRead;
}

CONNECTION_INL
(isize) write_to_server(Epoll &epoll) {
	isize bytesWritten = recvBuffer.write(writeFd, bodySize);
	if (bytesWritten < 0)
		return bytesWritten;
}

CONNECTION_INL
(isize) write_to_server_chunked(Epoll &epoll) {
	if (!epoll.is_readable())
		return 0;
	epoll.clr_read_flag();
	isize bytesRead = recvBuffer.read(clientFd, ATOMIC_IOSIZE);

	if (options & Options::CHUNKED_LENGTH) {
		bytesWritten = recvBuffer.decode(writeFd, chunkSize, bodySize);
		if (bytesWritten == -1)
			return flush_setup_close(epoll, Status::i400);
		if (bytesWritten == -3)
			return flush_setup_close(epoll, Status::i413);
	}
	return bytesRead == 0 ? -1 : bytesRead;
}

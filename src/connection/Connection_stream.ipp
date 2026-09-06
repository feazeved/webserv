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
	return sendBuffer.write(clientFd, ATOMIC_IOSIZE);
}

CONNECTION_INL
(isize) read_from_client(Epoll &epoll) {
	if (!epoll.is_readable())
		return 0;
	epoll.clr_read_flag();
	const isize bytesRead = recvBuffer.read(clientFd, ATOMIC_IOSIZE);
	if (bytesRead == -2) {
		const Status::Code code = mode <= Mode::PARSE ? Status::i431 : Status::i413;
		return flush_setup_close(epoll, code) < 0 ? -1 : 0;
	}
	return bytesRead == 0 ? -1 : bytesRead;
}

CONNECTION_INL
(Status::Code) write_chunked() {
	HTTP_Buffer tmpBuffer = {};
	Status::Code code = recvBuffer.dechunk(tmpBuffer, chunkSize, bodySize);
	if (code >= Status::i400)
		return code;

	const usize bytesToWrite = tmpBuffer.size();
	if (bytesToWrite <= 0)
		return code;
	if (tmpBuffer.write(writeFd, bytesToWrite) == -1)
		return Status::i500;
	return code;
}

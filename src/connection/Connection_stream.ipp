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
(isize) write_to_server() {
	isize bytesWritten = recvBuffer.write(writeFd, bodySize);
	if (bytesWritten < 0)
		return bytesWritten;
	bodySize -= (usize) bytesWritten;
	return bytesWritten;
}

// TODO: The buffer is only going to fill with data related to the chunks, decide if compaction is worth given current length
// Optimization opportunity here to have src copy directly to itself
// Otherwise just prepend the remainder to the end of what was read
CONNECTION_INL
(isize) write_to_server_chunked() {
	if (recvBuffer.readPos < recvBuffer.scanPos) {
		if (recvBuffer.write(writeFd, MIN(recvBuffer.scanPos - recvBuffer.readPos, (usize)ATOMIC_IOSIZE)) < 0)
			return -1;
		return 0;
	}

	HTTP_Buffer tmpBuffer = {};
	isize result = recvBuffer.dechunk(tmpBuffer, chunkSize, bodySize);
	if (result == -1)
		return -1;
	if (tmpBuffer.size() == 0) {
		recvBuffer.scanPos = recvBuffer.readPos;
		recvBuffer.compact();
		return result;
	}

	if (tmpBuffer.write(writeFd, ATOMIC_IOSIZE) < 0)
		return -1;

	const usize decodedRemaining = tmpBuffer.size();
	const usize rawRemaining = recvBuffer.size();
	if (rawRemaining != 0)
		tmpBuffer.append(recvBuffer.rptr(), rawRemaining);
	recvBuffer.bufcpy(tmpBuffer);
	recvBuffer.scanPos = decodedRemaining;
	return result;
}

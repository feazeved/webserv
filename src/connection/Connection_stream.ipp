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
	if (bytesRead <= 0) {
		if (bytesRead == -2)
			return flush_setup_close(epoll, Status::i413);
		return -1;
	}
	return bytesRead;
}

// CONNECTION_INL
// (Status::Code) write_to_server(HTTP_Buffer &src, usize bytes, bool isCgi) {
// 	bytes = MIN(bytes, ATOMIC_IOSIZE);
// 	isize bytesWritten = src.write(writeFd, bodySize);
// 	if (bytesWritten < 0)
// 		return isCgi ? Status::unset : Status::i500;
// 	bodySize -= (usize) bytesWritten;
// 	return Status::ok;
// }

// CONNECTION_INL
// (Status::Code) write_to_server_chunked(bool isCgi) {
// 	Status::Code code = Status::ok;

// 	if (recvBuffer.readPos < recvBuffer.scanPos) {
// 		const usize bytesLeft = recvBuffer.scanPos - recvBuffer.readPos;
// 		code = write_to_server(recvBuffer, bytesLeft, isCgi);
// 		if (code != Status::ok)
// 			return code;
// 		if (recvBuffer.readPos < recvBuffer.scanPos)
// 			return Status::unset;
// 	}

// 	HTTP_Buffer tmpBuffer = {};
// 	code = recvBuffer.dechunk(tmpBuffer, chunkSize, bodySize);
// 	if (tmpBuffer.size() != 0) {
// 		code = write_to_server(tmpBuffer, ATOMIC_IOSIZE, isCgi);
// 		const usize decodedRemaining = tmpBuffer.size();
// 		const usize rawRemaining = recvBuffer.size();
// 		if (rawRemaining != 0)
// 			tmpBuffer.append(recvBuffer.rptr(), rawRemaining);
// 		recvBuffer.bufcpy(tmpBuffer);
// 		recvBuffer.scanPos = decodedRemaining;
// 	}
// 	return code;
// }

#pragma once
#include "Connection.hpp"

CONNECTION_INL
(isize) download_file_fixed(Epoll &epoll) {
	isize bytesWritten = 0;
	if (bodySize != 0 && recvBuffer.size() != 0) {
		bytesWritten = recvBuffer.write(writeFd, bodySize);
		if (bytesWritten < 0)
			return flush_setup_close(epoll, Status::i500);
		bodySize -= (usize)bytesWritten;
	}
	if (bodySize == 0) {
		close(writeFd);
		writeFd = -1;
		bodySize = 0;
		build_header(Status::i201);
		return flush_setup(epoll, Status::i201);
	}
	if (recvBuffer.size() < bodySize)
		return read_from_client(epoll);
	return bytesWritten;
}

CONNECTION_INL
(isize) download_file_chunked(Epoll &epoll) {
	Status::Code code = write_chunked();
	if (code >= Status::i400)
		return flush_setup_close(epoll, code);
	if (code == Status::ok) {
		close(writeFd);
		writeFd = -1;
		bodySize = 0;
		build_header(Status::i201);
		return flush_setup(epoll, Status::i201);
	}
	return read_from_client(epoll);
}

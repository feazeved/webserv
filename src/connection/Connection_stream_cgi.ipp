#pragma once
#include "Connection.hpp"

CONNECTION_INL
(isize) switch_to_cgi(Epoll &epoll) {
	close(writeFd);
	writeFd = -1;
	mode = Mode::CGI;
	if (epoll.modify(clientFd, EPOLLOUT, epollState))
		return -1;
	return 0;
}

CONNECTION_INL
(isize) cgi_chunked(Epoll &epoll) {
	Status::Code code = write_chunked();
	if (code >= Status::i400)
		return flush_setup_close(epoll, code);
	if (code == Status::ok) {
		bodySize = 0;
		return switch_to_cgi(epoll);
	}
	return read_from_client(epoll);
}

CONNECTION_INL
(isize) cgi_fixed(Epoll &epoll) {
	isize bytesWritten = 0;
	if (bodySize != 0 && recvBuffer.size() != 0) {
		bytesWritten = recvBuffer.write(writeFd, bodySize);
		if (bytesWritten < 0)
			return flush_setup_close(epoll, Status::i500);
		bodySize -= (usize)bytesWritten;
	}
	if (bodySize == 0)
		return switch_to_cgi(epoll);
	if (recvBuffer.size() < bodySize)
		return read_from_client(epoll);
	return bytesWritten;
}

CONNECTION_INL
(isize) cgi(Epoll &epoll) {
	isize bytesRead = sendBuffer.read(readFd, ATOMIC_IOSIZE);
	if (bytesRead == 0) {
		close(readFd);
		readFd = -1;
	}
	Span header = sendBuffer.find_header_end();
	if (header.ptr == NULL) {
		if (bytesRead == -2 || readFd == -1)
			return flush_setup_close(epoll, Status::i500);
		return 0;	// Still no CGI Header
	}
	Status::Code code = build_cgi_header(Status::i200);
	if (code == Status::ixxx)
		return flush_setup_close(epoll, Status::i500);
	if (readFd == -1)
		return flush_setup(epoll);
	mode = Mode::CGI_PARSED;
	return write_to_client(epoll);
}

CONNECTION_INL
(isize) cgi_parsed(Epoll &epoll) {
	isize bytesRead = sendBuffer.read(readFd, ATOMIC_IOSIZE);
	if (bytesRead == 0)
		return flush_setup(epoll);
	return write_to_client(epoll);
}

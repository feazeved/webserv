#pragma once
#include "Connection.hpp"

CONNECTION_INL
(isize) write_to_client(Epoll &epoll) {
	if (!epoll.is_writeable())
		return epoll.set_write(clientFd, 1);

	isize bytesWritten = sendBuffer.write(clientFd, ATOMIC_IOSIZE);
	if (bytesWritten <= 0)
		return close_connection(false);
	if (sendBuffer.size() != 0)
		return bytesWritten;
	if (mode == Mode::FLUSH) {
		mode = Mode::PARSE;
		return bytesWritten;
	}
	return close_connection();
}

CONNECTION_INL
(isize) read_from_client(Epoll &epoll) {
	if (!epoll.is_readable())
		return epoll.set_read(clientFd, 1);

	isize bytesRead = recvBuffer.read(clientFd, ATOMIC_IOSIZE);
	if (bytesRead <= 0)
		return close_connection(false);
	return bytesRead;
}

CONNECTION_INL
(isize) download_file(Epoll &epoll) {
	isize bytesWritten;

	if (options & Options::CHUNKED_LENGTH)
		bytesWritten = recvBuffer.decode(writeFd, chunkSize, bodySize);
	else {
		bytesWritten = recvBuffer.write(writeFd, bodySize);
		if (bytesWritten > 0)
			bodySize -= (usize) bytesWritten;
	}

	if (bytesWritten == -1) {
		close(writeFd);
		writeFd = -1;
		status = Status::i500;
		return close_connection();
	}

	if (!status.is_set() && bodySize == 0) {	// Must guarantee that bodySize is 0
		close(writeFd);
		writeFd = -1;	// Finished reading
		status = Status::i201;
		build_header();
		bool keepAlive = !!(options & Options::KEEP_ALIVE);
		mode = keepAlive ? Mode::FLUSH : Mode::CLOSE;
	}
	return write_to_client(epoll);
}

/*	The pipe fds here are configured to be non-blocking and read/write errors are ignored
	Failure conditions for these fds are instead handled by CGI timeouts
*/

CONNECTION_INL
(isize) cgi_method() {
	isize bytesWritten, bytesRead;

	if (options & Options::CHUNKED_LENGTH)
		bytesWritten = recvBuffer.decode(writeFd, chunkSize, bodySize);
	else {
		bytesWritten = recvBuffer.write(writeFd, bodySize);
		if (bytesWritten > 0)
			bodySize -= (usize) bytesWritten;
	}

	if (bytesWritten == -1) {
		close(writeFd);
		writeFd = -1;
		status = Status::i500;
		return close_connection();
	}

	if (bodySize == 0) {	// Must guarantee that bodySize is 0
		close(writeFd);
		writeFd = -1;	// Finished reading
	}

	bytesRead = sendBuffer.read(readFd, ATOMIC_IOSIZE);
	if (bytesRead == 0) {
		close(readFd);
		readFd = -1;
	}

	isize delta = ((bytesWritten < 0 || bytesRead < 0) ? -1 : 1);

	if (!status.is_set()) {
		if (sendBuffer.find_header_end() != SIZE_MAX) {
			if (sendBuffer.is_full())
				return -1;
			return 0;	// Still no CGI Header
		}
		build_cgi_header();
	}
	return bytesRead;
}

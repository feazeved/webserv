#pragma once
#include "Connection.hpp"

CONNECTION_INL
(isize) write_to_client(Epoll &epoll) {
	if (!epoll.is_writeable())
		return epoll.set_write(clientFd, 1);

	isize bytesWritten = sendBuffer.write(clientFd, ATOMIC_IOSIZE);
	return bytesWritten;
}

CONNECTION_INL
(isize) read_from_client(Epoll &epoll) {
	if (!epoll.is_readable())
		return epoll.set_read(clientFd, 1);

	isize bytesRead = recvBuffer.read(clientFd, ATOMIC_IOSIZE);
	return bytesRead;
}

CONNECTION_INL
(isize) flush(Epoll &epoll) {
	isize bytesWritten = write_to_client(epoll);
	if (sendBuffer.size() > 0)
		return bytesWritten;
	options = 0;
	contentType = Mime::OCTET_STREAM;
	bodySize = 0;
	chunkSize = SIZE_MAX;
	mode = Mode::PARSE;
	req.clear();
	sendBuffer.clear();
	status.clear();
	startTime = Clock::time_elapsed();
	return bytesWritten;
}

CONNECTION_INL
(isize) download_file(Epoll &epoll) {
	isize bytesWritten = 0;
	if (read_from_client(epoll) == -1)
		return -1;
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
		bool isChunked = options & Options::CHUNKED_LENGTH;
		Status::Code code = isChunked ? Status::i400 : Status::i500;
		return flush_setup_close(epoll, code);
	}

	if (!status.is_set() && bodySize == 0) {	// Must guarantee that bodySize is 0
		close(writeFd);
		writeFd = -1;	// Finished reading
		build_header();
		return flush_setup(epoll, Status::i201);
	}
	return write_to_client(epoll);
}
/*	The pipe fds here are configured to be non-blocking and read/write errors are ignored
	Failure conditions for these fds are instead handled by CGI timeouts
*/

CONNECTION_INL
(isize) cgi_method(Epoll &epoll) {
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
		return -1;
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

	// isize delta = ((bytesWritten < 0 || bytesRead < 0) ? -1 : 1);

	if (!status.is_set()) {
		if (sendBuffer.find_header_end() != SIZE_MAX) {
			if (sendBuffer.is_full())
				return -1;
			return 0;	// Still no CGI Header
		}
		build_cgi_header();
		flush_setup(epoll, (Status::Code) status.index);
	}
	return bytesRead;
}

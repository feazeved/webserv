#pragma once
#include "Connection.hpp"

CONNECTION_INL
(isize) upload_file(u32 events) {
	isize bytesRead = sendBuffer.read(readFd, ATOMIC_IOSIZE);
	if (bytesRead == 0) {
		close(readFd);
		readFd = -1;
		bool keepAlive = !!(options & Options::CONNECTION_TYPE);
		mode = keepAlive ? Mode::FLUSH : Mode::CLOSE;
	}
	else if (bytesRead == -1) {
		close(readFd);
		readFd = -1;
		mode = Mode::CLOSE;
		return error_path();
	}
	return write_to_client(events);
}

CONNECTION_INL
(isize) download_file(u32 events) {
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
		return error_path();
	}

	if (!status.is_set() && bodySize == 0) {	// Must guarantee that bodySize is 0
		close(writeFd);
		writeFd = -1;	// Finished reading
		status = Status::i201;
		build_header();
		bool keepAlive = !!(options & Options::CONNECTION_TYPE);
		mode = keepAlive ? Mode::FLUSH : Mode::CLOSE;
	}
	return write_to_client(events);
}

/*
	The pipe fds here are configured to be non-blocking and read/write errors are ignored
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
		return error_path();
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
	// bonusTime = CLAMP(bonusTime + delta, 0, 30);

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

#pragma once
#include "Connection.hpp"

CONNECTION_INL
(isize) flush_setup(Epoll &epoll, Status::Code code) {
	status = code;
	clear();
	mode = Mode::FLUSH;
	isize bytesWritten = write_to_client(epoll);
	if (sendBuffer.size() > 0 && epoll.clr_read(clientFd))	// Given that it would start parsing after, doesnt make sense to turn off read
		options &= ~(u16)Options::KEEP_ALIVE;
	return bytesWritten;		// Keep the connection alive until header is flushed
}

CONNECTION_INL
(isize) flush_setup_close(Epoll &epoll, Status::Code code) {
	status = code;
	clear();
	options &= ~(u16)Options::KEEP_ALIVE;
	mode = Mode::FLUSH;
	build_error_header();
	if (epoll.clr_read(clientFd))
		return -1;
	return write_to_client(epoll);		// Keep the connection alive until header is flushed
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
	if (epoll.clr_write(clientFd))
		return -1;
	return bytesWritten;
}

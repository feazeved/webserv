#pragma once
#include "Connection.hpp"

CONNECTION_INL
(isize) flush_setup(Epoll &epoll, Status::Code code) {
	ASSERT(status.is_set(), "Status was not set in close connection");
	clear();
	mode = Mode::FLUSH;
	return write_to_client(epoll);		// Keep the connection alive until header is flushed
}

CONNECTION_INL
(isize) flush_setup_close(Epoll &epoll, Status::Code code) {
	ASSERT(status.is_set(), "Status was not set in close connection");
	clear();
	options &= ~(u16)Options::KEEP_ALIVE;
	mode = Mode::FLUSH;
	build_error_header();
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
	return bytesWritten;
}

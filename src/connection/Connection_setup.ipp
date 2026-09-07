#pragma once
#include "Connection.hpp"

CONNECTION_INL
(isize) del_setup(Epoll &epoll) {
	Buffer64 pathBuffer = {};
	append_target_path(pathBuffer);

	struct stat st;
	if (stat(pathBuffer, &st) == -1)
		return flush_setup_close(epoll, s_get_status());
	if (S_ISDIR(st.st_mode))
		return flush_setup_close(epoll, Status::i403);	// Forbids deleting directories
	if (unlink(pathBuffer) == -1)
		return flush_setup_close(epoll, s_get_status());
	build_header(Status::i204);
	return flush_setup(epoll, Status::i204);
}

CONNECTION_INL
(isize) post_setup(Epoll &epoll) {
	Buffer64 pathBuffer = {};
	const Span uploadStore = req.location->get_upload_store();
	pathBuffer.append(uploadStore);
	pathBuffer.append(req.relativeTarget);
	*pathBuffer = 0;

	writeFd = open(pathBuffer, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NONBLOCK, 0644);
	if (writeFd == -1)
		return flush_setup_close(epoll, s_get_status());
	if (mode == Mode::POST_FIXED)
		return download_file_fixed(epoll);
	return download_file_chunked(epoll);
}

CONNECTION_INL
(isize) redirect_setup(Epoll &epoll, Status::Code code) {
	status = code;
	bodySize = 0;
	options &= ~(u16)Options::KEEP_ALIVE;
	mode = Mode::FLUSH;
	sendBuffer.append("HTTP/1.1 ");
	sendBuffer.append(status.status_str());
	sendBuffer.append("\r\nLocation: ");
	sendBuffer.append(req.location->get_redirect_target());
	sendBuffer.append("\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");
	if (epoll.modify(clientFd, EPOLLOUT, epollState))
		return -1;
	return write_to_client(epoll);
}

CONNECTION_INL
(isize) parse_setup(Epoll &epoll) {
	recvBuffer.compact();	// REVIEW: Check this
	options = 0;
	contentType = Mime::OCTET_STREAM;
	bodySize = 0;
	chunkSize = 0;
	mode = Mode::PARSE_FIRST;
	req.clear();
	sendBuffer.clear();
	status.clear();
	startTime = Clock::time_elapsed();
	if (epoll.modify(clientFd, EPOLLIN, epollState))
		return -1;
	return parse_first(epoll);		// Keep the connection alive until header is flushed
}

CONNECTION_INL
(isize) flush_setup(Epoll &epoll, Status::Code code) {
	status = code;
	clear();
	mode = Mode::FLUSH;
	isize bytesWritten = write_to_client(epoll);
	if (sendBuffer.size() > 0 && epoll.modify(clientFd, EPOLLOUT, epollState))
		return -1;	// TODO: See if i can't just stream the output then close
	return bytesWritten;
}

CONNECTION_INL
(isize) flush_setup_close(Epoll &epoll, Status::Code code) {
	status = code;
	clear();
	options &= ~(u16)Options::KEEP_ALIVE;
	mode = Mode::FLUSH;
	build_error_header(code);
	if (epoll.modify(clientFd, EPOLLOUT, epollState))
		return -1;
	return write_to_client(epoll);
}

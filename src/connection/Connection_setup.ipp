#pragma once
#include "Connection.hpp"

CONNECTION_INL
(isize) del_setup(Epoll &epoll) {
	Buffer64 pathBuffer = {};
	append_target_path(pathBuffer);
	s_switch_to_streaming(recvBuffer, parseBuffer);

	struct stat st;
	if (stat(pathBuffer, &st) == -1)
		return flush_setup_close(epoll, s_get_status());
	if (S_ISDIR(st.st_mode))
		return flush_setup_close(epoll, Status::i403);	// Forbids deleting directories
	if (unlink(pathBuffer) == -1)
		return flush_setup_close(epoll, s_get_status());
	build_header(Status::i204);
	return flush_setup(epoll);
}

CONNECTION_INL
(isize) post_setup(Epoll &epoll) {
	Buffer64 pathBuffer = {};
	const Span uploadStore = req.location->get_upload_store();
	pathBuffer.append(uploadStore);
	pathBuffer.append(req.relativeTarget);
	*pathBuffer = 0;
	s_switch_to_streaming(recvBuffer, parseBuffer);

	writeFd = open(pathBuffer, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NONBLOCK, 0644);
	if (writeFd == -1)
		return flush_setup_close(epoll, s_get_status());
	if (mode == Mode::POST_FIXED)
		return download_file_fixed(epoll);
	return download_file_chunked(epoll);
}

CONNECTION_INL
(isize) redirect_setup(Epoll &epoll, Status::Code code) {
	bodySize = 0;
	options &= ~(u16)Options::KEEP_ALIVE;
	mode = Mode::FLUSH;
	sendBuffer.append("HTTP/1.1 ");
	sendBuffer.append(Status::s_status_str(code));
	sendBuffer.append("\r\nLocation: ");
	sendBuffer.append(req.location->get_redirect_target());
	sendBuffer.append("\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");
	if (epoll.modify(clientFd, EPOLLOUT, epollState))
		return -1;
	return write_to_client(epoll);
}

CONNECTION_INL
(isize) parse_setup(Epoll &epoll) {
	s_switch_to_parsing(recvBuffer, parseBuffer);
	options = 0;
	contentType = Mime::OCTET_STREAM;
	bodySize = 0;
	chunkSize = 0;
	mode = Mode::PARSE_FIRST;
	req.clear();
	sendBuffer.clear();
	startTime = Clock::time_elapsed();
	if (epoll.modify(clientFd, EPOLLIN, epollState))
		return -1;
	return parse_first(epoll);		// Keep the connection alive until header is flushed
}

CONNECTION_INL
(isize) flush_setup(Epoll &epoll) {
	clear();
	mode = Mode::FLUSH;
	isize bytesWritten = write_to_client(epoll);
	if (sendBuffer.size() > 0 && epoll.modify(clientFd, EPOLLOUT, epollState))
		return -1;	// TODO: See if i can't just stream the output then close
	return bytesWritten;
}

// Flush_close only needs to know the Status
CONNECTION_INL
(isize) flush_setup_close(Epoll &epoll, Status::Code code) {
	clear();
	options &= ~(u16)Options::KEEP_ALIVE;
	mode = Mode::FLUSH;
	build_error_header(code);	// Already calls sendBuffer.clear()
	if (epoll.modify(clientFd, EPOLLOUT, epollState))
		return -1;
	return write_to_client(epoll);
}

#pragma once
#include "Connection.hpp"

CONNECTION_INL
(isize) download_file_chunked(Epoll &epoll) {
	isize result = write_to_server_chunked();
	if (result == -1)
		return -1;
	if (result == 1) {
		bodySize = recvBuffer.scanPos - recvBuffer.readPos;
		mode = Mode::POST;
		if (epoll.modify(clientFd, EPOLLOUT, epollState))
			return -1;
		return download_file(epoll);
	}
	if (read_from_client(epoll) == -1)
		return -1;
	return 0;
}

CONNECTION_INL
(isize) download_file_fixed(Epoll &epoll) {
	isize bytesWritten = write_to_server();
	if (bytesWritten < 0)
		return flush_setup_close(epoll, Status::i500);
	if (recvBuffer.size() < bodySize && read_from_client(epoll) == -1)
		return -1;
	if (recvBuffer.size() >= bodySize) {
		mode = Mode::POST;
		if (epoll.modify(clientFd, EPOLLOUT, epollState))
			return -1;
		return download_file(epoll);
	}
	return bytesWritten;
}

CONNECTION_INL
(isize) download_file(Epoll &epoll) {
	isize bytesWritten = write_to_server();
	if (bytesWritten < 0)
		return flush_setup_close(epoll, Status::i500);
	if (bodySize == 0) {
		close(writeFd);
		writeFd = -1;	// Finished reading
		build_header(Status::i201);
		return flush_setup(epoll, Status::i201);
	}
	return bytesWritten;
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

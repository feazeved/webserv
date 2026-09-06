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

#pragma once
#include "Connection.hpp"

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
	return download_file(epoll);
}

CONNECTION_INL
(isize) download_file(Epoll &epoll) {
	isize bytesWritten = 0;
	isize bytesRead = read_from_client(epoll);
	if (bytesRead == -1)
		return -1;
	if (options & Options::CHUNKED_LENGTH)
		bytesWritten = recvBuffer.decode(writeFd, chunkSize, bodySize);
	else {
		bytesWritten = recvBuffer.write(writeFd, bodySize);
		if (bytesWritten > 0)
			bodySize -= (usize) bytesWritten;
	}

	if (bytesWritten == -1) {
		bool isChunked = options & Options::CHUNKED_LENGTH;
		Status::Code code = isChunked ? Status::i400 : Status::i500;
		return flush_setup_close(epoll, code);
	}

	if (!status.is_set() && bodySize == 0) {
		close(writeFd);
		writeFd = -1;	// Finished reading
		build_header(Status::i201);
		return flush_setup(epoll, Status::i201);
	}
	return write_to_client(epoll);
}

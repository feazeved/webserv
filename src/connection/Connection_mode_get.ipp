#pragma once
#include "Connection.hpp"

// Finished state means everything is read to the send buffer and it only needs flushing of the send buffer
CONNECTION_INL
(isize) upload_file(Epoll &epoll) {
	isize bytesRead = sendBuffer.read(readFd, ATOMIC_IOSIZE);
	if (bytesRead <= 0 && (bytesRead == -1 || bodySize != 0))
		return -1;
	bodySize -= (usize)bytesRead;
	if (bodySize == 0) {
		close(readFd);
		readFd = -1;
		return flush_setup(epoll, Status::i200);
	}
	return write_to_client(epoll);
}

CONNECTION_INL
(isize) get_setup(Epoll &epoll) {
	Buffer64 pathBuffer = {};
	append_target_path(pathBuffer);

	struct stat st;
	if (stat(pathBuffer, &st) == -1)
		return flush_setup_close(epoll, s_get_status());

	if (S_ISDIR(st.st_mode))
		return get_directory_setup(epoll, st, pathBuffer);
	readFd = open(pathBuffer, O_RDONLY | O_CLOEXEC | O_NONBLOCK);
	if (readFd == -1)
		return flush_setup_close(epoll, s_get_status());
	bodySize = (usize)st.st_size;
	contentType = fn::match_mime(pathBuffer.get_span());
	build_header(Status::i200);
	return upload_file(epoll);
}

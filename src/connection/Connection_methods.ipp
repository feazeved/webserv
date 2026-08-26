#pragma once
#include "Connection.hpp"

CONNECTION_INL
(isize) del_first_run() {
	Buffer16 pathBuffer;

	s_build_path(pathBuffer, req.path, req.location->root);

	struct stat st;
	if (stat(pathBuffer, &st) == -1)
		return s_get_status(status);

	if (S_ISDIR(st.st_mode))
		return s_get_status(status);	// Forbids deleting directories

	if (unlink(pathBuffer) == -1)
		return s_get_status(status);

	status = Status::i204;
	build_header();
	return 0;
}

CONNECTION_INL
(isize) post_first_run() {
	Buffer16 pathBuffer;
	Span storePath = req.location->uploadStore.extract();
	s_build_path(pathBuffer, storePath, req.location->root);

	writeFd = open(pathBuffer, O_WRONLY | O_CREAT | O_EXCL, 0644);
	if (writeFd == -1) {
		mode = Mode::CLOSE;
		return s_get_status(status);
	}
	return 0;
}

CONNECTION_INL
(isize) get_first_run() {
	Buffer16 pathBuffer;

	s_build_path(pathBuffer, req.path, req.location->root);

	struct stat st;
	if (stat(pathBuffer, &st) == -1)
		return s_get_status(status);

	if (S_ISDIR(st.st_mode))
		return get_directory(st, pathBuffer);

	int	rawFd = open(pathBuffer, O_RDONLY);
	if (rawFd == -1)
		return s_get_status(status);

	readFd = rawFd;
	status = Status::i200;
	bodySize = (usize)st.st_size;
	build_header();
	return 0;
}

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

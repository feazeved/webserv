#pragma once
#include "Connection.hpp"

CONNECTION_INL
(isize) del_first_run() {
	Buffer16 pathBuffer;
	Location &loc = cfg->locations[request.locationIndex];
	Span path = request.path.extract((char*)recvBuffer.data);

	s_build_path(pathBuffer, path, loc.root);

	struct stat st;
	if (stat(pathBuffer, &st) == -1)
		return s_get_status(request.status);

	if (S_ISDIR(st.st_mode))
		return s_get_status(request.status);	// Forbids deleting directories

	if (unlink(pathBuffer) == -1)
		return s_get_status(request.status);

	request.status = Status::i204;
	build_header();
	return 0;
}

CONNECTION_INL
(isize) post_first_run() {
	Buffer16 pathBuffer;
	Location &loc = cfg->locations[request.locationIndex];
	Span storePath = loc.uploadStore.extract();
	s_build_path(pathBuffer, storePath, loc.root);

	writeFd = open(pathBuffer, O_WRONLY | O_CREAT | O_EXCL, 0644);
	if (writeFd == -1) {
		mode = Mode::CLOSE;
		return s_get_status(request.status);
	}
	return 0;
}

CONNECTION_INL
(isize) get_first_run() {
	Buffer16 pathBuffer;
	Location &loc = cfg->locations[request.locationIndex];
	Span path = request.path.extract((char*)recvBuffer.data);

	s_build_path(pathBuffer, path, loc.root);

	struct stat st;
	if (stat(pathBuffer, &st) == -1)
		return s_get_status(request.status);

	if (S_ISDIR(st.st_mode))
		return get_directory(st, pathBuffer);

	int	rawFd = open(pathBuffer, O_RDONLY);
	if (rawFd == -1)
		return s_get_status(request.status);

	readFd = rawFd;
	request.status = Status::i200;
	request.bodySize = (usize)st.st_size;
	build_header();
	return 0;
}

CONNECTION_INL
(isize) upload_file(u32 events) {
	isize bytesRead = sendBuffer.read(readFd, ATOMIC_IOSIZE);
	if (bytesRead == 0) {
		close(readFd);
		readFd = -1;
		bool keepAlive = !!(request.options & Options::CONNECTION_TYPE);
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

	if (request.options & Options::CHUNKED_LENGTH)
		bytesWritten = recvBuffer.decode(writeFd, request.chunkSize, request.bodySize);
	else {
		bytesWritten = recvBuffer.write(writeFd, request.bodySize);
		if (bytesWritten > 0)
			request.bodySize -= (usize) bytesWritten;
	}

	if (bytesWritten == -1) {
		close(writeFd);
		writeFd = -1;
		request.status = Status::i500;
		return error_path();
	}

	if (!request.status.is_set() && request.bodySize == 0) {	// Must guarantee that bodySize is 0
		close(writeFd);
		writeFd = -1;	// Finished reading
		request.status = Status::i201;
		build_header();
		bool keepAlive = !!(request.options & Options::CONNECTION_TYPE);
		mode = keepAlive ? Mode::FLUSH : Mode::CLOSE;
	}
	return write_to_client(events);
}

#pragma once
#include <sys/stat.h>

#include "Connection.hpp"
#include "Connection_helpers.ipp"

namespace HTTP {

CONNECTION_INL
(isize) del_first_run() {
	char pathBuffer[8192];
	char* ptr = request.path.index + (char*) recvBuffer.data;

	build_path(pathBuffer, ptr, request.path.size);

	static struct stat st;
	if (stat(pathBuffer, &st) == -1)
		return s_get_status(request.status);

	if (S_ISDIR(st.st_mode))
		return s_get_status(request.status);

	if (unlink(pathBuffer) == -1)
		return s_get_status(request.status);

	request.status = Status::i204;
	build_header();
	return 0;
}

CONNECTION_INL
(isize) post_first_run() {
	char pathBuffer[8192];
	Location &loc = cfg->locations[request.locationIndex];
	const StringView32 &storePath = loc.uploadStore;

	s_build_path(pathBuffer, storePath.kptr(), storePath.length, loc.root);

	writeFd = open(storePath.kptr(), O_WRONLY | O_CREAT | O_EXCL, 0644);
	if (writeFd == -1) {
		mode = Mode::CLOSE;
		return s_get_status(request.status);
	}
	return 0;
}

CONNECTION_INL
(isize) get_first_run() {
	char pathBuffer[8192];
	char* ptr = request.path.index + (char*) recvBuffer.data;
	Location &loc = cfg->locations[request.locationIndex];

	s_build_path(pathBuffer, ptr, request.path.size, loc.root);

	struct stat st;
	if (stat(pathBuffer, &st) == -1)
		return s_get_status(request.status);

	if (S_ISDIR(st.st_mode))
		return get_directory(&st);

	i32	rawFd = open(pathBuffer, O_RDONLY);
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
		mode = Mode::FLUSHING;
	}
	else if (bytesRead == -1) {
		close(readFd);
		readFd = -1;
		mode = Mode::CLOSE;
		return -1;
	}
	if (write_to_client(events) == -1)
		return -1;
	return bytesRead;
}

CONNECTION_INL
(isize) download_file() {
	isize bytesWritten;

	if (request.options & Options::CHUNKED_LENGTH)
		bytesWritten = decode();
	else {
		bytesWritten = recvBuffer.write(writeFd, request.bodySize);
		if (bytesWritten > 0)
			request.bodySize -= (usize) bytesWritten;
	}

	if (!request.status.is_set() && request.bodySize == 0) {	// Must guarantee that bodySize is 0
		close(writeFd);
		writeFd = -1;	// Finished reading
		request.status = Status::i201;
		build_header();
		mode = Mode::FLUSHING;
	}

	return bytesWritten;
}

// HTTP namespace
}

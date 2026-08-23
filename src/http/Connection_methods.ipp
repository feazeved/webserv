#pragma once
#include <sys/stat.h>

#include "Connection.hpp"
#include "Connection_helpers.ipp"
namespace HTTP {

CONNECTION_INL
(isize) del_first_run() {
	char pathBuffer[8192];
	char* ptr = request.path.index + (char*) recvBuffer.cursor.memStart;

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

	build_path(pathBuffer, storePath.c_str(), storePath.length);

	int	rawFd = open(storePath.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0644);
	if (rawFd == -1)
		return s_get_status(request.status);

	writeFd = rawFd;
	return 0;
}

CONNECTION_INL
(isize) get_first_run() {
	char pathBuffer[8192];
	char* ptr = request.path.index + (char*) recvBuffer.cursor.memStart;

	build_path(pathBuffer, ptr, request.path.size);

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
	state |= State::WRITING_TO_CLIENT;
	return 0;
}

// Header will already be built in the configure function
CONNECTION_INL
(isize) get_method() {
	if (readFd == -1)
		return 0;
	return read_from_server();
}

CONNECTION_INL
(isize) post_method() {
	isize bytesWritten = write_to_server();
	if (bytesWritten < 0)
		return bytesWritten;

	if (!request.status.is_set() && writeFd == -1) {
		request.status = Status::i201;
		build_header();
	}
	return bytesWritten;
}

CONNECTION_INL
(isize) del_method() {
	return 0;
}

// HTTP namespace
}

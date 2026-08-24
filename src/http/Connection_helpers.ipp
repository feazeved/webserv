#pragma once
#include <sys/stat.h>
#include <errno.h>

#include "Buffer.hpp"
#include "core.hpp"
#include "HTTP.hpp"

namespace HTTP {

static inline
isize s_get_status(Status &status) {
	const int error = errno;

	if (error == ENOENT || error == ENOTDIR)
		status = Status::i404;
	if (error == EACCES || error == EPERM || error == EROFS)
		status = Status::i403;
	if (error == EEXIST || error == ENOTEMPTY || error == EBUSY)
		status = Status::i409;
	if (error == ENAMETOOLONG)
		status = Status::i414;
	if (error == ENOSPC || error == EDQUOT)
		status = Status::i507;
	if (error == EMFILE || error == ENFILE || error == ENOMEM)
		status = Status::i503;
	status = Status::i500;
	return -1;
}

static inline
void s_build_path(char* buffer, const char *ptr, usize length, StringView32& root) {
	// const StringView32& root = cfg->locations[request.locationIndex].root;

	MEMCPY(buffer, root.c_str(), root.length);
	buffer += root.length;
	MEMCPY(buffer, ptr, length);
	buffer[length] = 0;
}


// Check epoll, see if can write, if not, set to write and return 0
static inline
isize s_write_to_client(HTTP_Buffer &buffer, int fd, u32 events) {
	isize bytesWritten = buffer.write(fd, ATOMIC_IOSIZE);
	if (bytesWritten < 0)
		return bytesWritten;
	return bytesWritten;
}

// Check epoll, see if can read, if not, set to write and return 0
static inline
isize s_read_from_client(HTTP_Buffer &buffer, int fd, u32 events) {
	isize bytesRead = buffer.read(fd, ATOMIC_IOSIZE);
	if (bytesRead < 0)
		return bytesRead;
	return bytesRead;
}

}

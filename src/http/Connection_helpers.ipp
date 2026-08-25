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
	MEMCPY(buffer, root.kptr(), root.length);
	buffer += root.length;
	MEMCPY(buffer, ptr, length);
	buffer[length] = 0;
}

}

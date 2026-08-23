#pragma once
#include <sys/stat.h>
#include <errno.h>

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

// static inline
// struct stat* s_check_path(char* buffer, Status &status) {
// 	static struct stat st;

// 	if (stat(buffer, &st) == -1) {
// 		status = (errno == ENOENT) ? Status::i404 : Status::i500;
// 		return &st;
// 	}
// 	return NULL;
// }

}

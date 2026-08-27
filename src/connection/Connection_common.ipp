#pragma once
#include "Connection.hpp"

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
pid_t s_exec_script(char *const argv[3], char **envp, int fdIn[2], int fdOut[2]) {
	bool fail = dup2(STDOUT_FILENO, fdOut[1]) == -1 || 
				dup2(STDIN_FILENO, fdIn[0]) == -1;

	close(fdOut[0]);	// Child Read End
	close(fdOut[1]);	// Parent Write End
	close(fdIn[0]);		// Child Read End
	close(fdIn[1]);		// Parent Write End
	if (fail) {
		close(STDOUT_FILENO);
		close(STDIN_FILENO);
		_exit(1);	// TODO: Appropriate return
	}

	execve(argv[0], argv, envp);
	if (errno != ENOENT && errno != ENOTDIR)
		_exit(126);
	_exit(127);
}

// Check epoll, see if can write, if not, set to write and return 0
CONNECTION_INL
(isize) write_to_client(u32 events) {
	isize bytesWritten = recvBuffer.write(clientFd, ATOMIC_IOSIZE);
	if (bytesWritten <= 0)
		return close_connection();
	if (sendBuffer.size() != 0)
		return bytesWritten;
	if (mode == Mode::FLUSH) {
		mode = Mode::PARSE;
		return bytesWritten;
	}
	return close_connection();
}

/*
	Check epoll, see if can write, if not, set to write and return 0
	This is only called when information is needed, therefore if is 0 bytes
	are read, the connection should close. But maybe it's error_path instead
*/
CONNECTION_INL
(isize) read_from_client(u32 events) {
	isize bytesRead = recvBuffer.read(clientFd, ATOMIC_IOSIZE);
	if (bytesRead <= 0)
		return close_connection();
	return bytesRead;
}

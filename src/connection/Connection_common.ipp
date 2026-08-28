#pragma once
#include "Connection.hpp"

static inline
isize s_get_status(Status &status) {
	const int error = errno;

	if (error == ENOENT || error == ENOTDIR)
		status = Status::i404;
	else if (error == EACCES || error == EPERM || error == EROFS)
		status = Status::i403;
	else if (error == EEXIST || error == ENOTEMPTY || error == EBUSY)
		status = Status::i409;
	else if (error == ENAMETOOLONG)
		status = Status::i414;
	else if (error == ENOSPC || error == EDQUOT)
		status = Status::i507;
	else if (error == EMFILE || error == ENFILE || error == ENOMEM)
		status = Status::i503;
	else
		status = Status::i500;
	return -1;
}

static inline
pid_t s_exec_script(char *const argv[3], char **envp, int fdIn[2], int fdOut[2]) {
	char *lastSlash = NULL;
	for (char *cursor = argv[1]; *cursor != '\0'; cursor++) {
		if (*cursor == '/')
			lastSlash = cursor;
	}
	if (lastSlash != NULL) {
		char workingDirectory[MAX_PATH_SIZE];
		usize length = (usize)(lastSlash - argv[1]);		// TODO: Review this
		if (length == 0)
			length = 1;
		MEMCPY(workingDirectory, argv[1], length);
		workingDirectory[length] = '\0';
		if (chdir(workingDirectory) == -1)
			std::exit(1);
	}

	bool fail = dup2(fdOut[1], STDOUT_FILENO) == -1 ||
				dup2(fdIn[0], STDIN_FILENO) == -1;

	close(fdOut[0]);	// Child Read End
	close(fdOut[1]);	// Parent Write End
	close(fdIn[0]);		// Child Read End
	close(fdIn[1]);		// Parent Write End
	if (fail) {
		close(STDOUT_FILENO);
		close(STDIN_FILENO);
		std::exit(1);
	}

	execve(argv[0], argv, envp);
	if (errno != ENOENT && errno != ENOTDIR)
		std::exit(126);
	std::exit(127);
}

// Check epoll, see if can write, if not, set to write and return 0
CONNECTION_INL
(isize) write_to_client(u32 events) {
	// if (sendBuffer.size() == 0)
	// 	return 0;
	// if ((events & EPOLLOUT) == 0)
	// 	return EPOLLOUT;
	isize bytesWritten = sendBuffer.write(clientFd, ATOMIC_IOSIZE);
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
	// if ((events & EPOLLIN) == 0)
	// 	return -2;
CONNECTION_INL
(isize) read_from_client(u32 events) {
	isize bytesRead = recvBuffer.read(clientFd, ATOMIC_IOSIZE);
	if (bytesRead <= 0)
		return close_connection();
	return bytesRead;
}

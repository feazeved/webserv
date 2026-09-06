#pragma once
#include "Connection.hpp"

/*
	A mode is the state that the connection is in. It's an exclusive variable, not a bitfield
	The connection starts in parse mode. When it is done parsing, it calls:

	Setup configures each mode and calls the execution of a method.
	When a method is done (GET, POST, CGI, AUTOINDEX) or when a non fatal error happens, it enters Flush mode

	Flush mode writes all the remaining bytes in sendBuffer, then either closes or goes back to parsing mode
	Whether it closes or not depends on if there was an error, or if it specified a keep-alive option
*/
CONNECTION_INL
(isize) dispatch(Epoll &epoll) {
	switch (mode) {
		case Mode::PARSE_FIRST:		return parse_first(epoll); break;
		case Mode::PARSE:			return parse(epoll); break;
		case Mode::GET:				return upload_file(epoll); break;
		case Mode::POST_FIXED:		return download_file_fixed(epoll); break;
		case Mode::POST_CHUNKED:	return download_file_chunked(epoll); break;
		case Mode::FLUSH:			return flush(epoll); break;
		case Mode::CGI:				return cgi(epoll); break;
		case Mode::CGI_FIXED:		return cgi_fixed(epoll); break;
		case Mode::CGI_CHUNKED:		return cgi_chunked(epoll); break;
		case Mode::AUTOINDEX:		return upload_directory(epoll); break;
		default: return -1;
	}
}

CONNECTION_INL
(char*) append_target_path(Buffer64 &buffer) {
	const Span root = req.location->get_root();
	const Span uri = req.location->get_uri();

	char* fullPath = buffer.append(root);
	const Span suffix = {req.target.ptr + uri.size, req.target.size - uri.size};
	if (suffix.size != 0) {
		const bool rootHasSlash = root.size != 0 && root.ptr[root.size - 1] == '/';
		const bool suffixHasSlash = suffix.ptr[0] == '/';
		if (!rootHasSlash && !suffixHasSlash)
			buffer.append("/");
		buffer.append(suffix.ptr + (rootHasSlash && suffixHasSlash), suffix.size - (rootHasSlash && suffixHasSlash));
	}
	*buffer = 0;
	return fullPath;
}

static inline
Status::Code s_get_status() {
	Status::Code code;
	const int error = errno;

	if (error == ENOENT || error == ENOTDIR)
		code = Status::i404;
	else if (error == EACCES || error == EPERM || error == EROFS)
		code = Status::i403;
	else if (error == EEXIST || error == ENOTEMPTY || error == EBUSY)
		code = Status::i409;
	else if (error == ENAMETOOLONG)
		code = Status::i414;
	else if (error == ENOSPC || error == EDQUOT)
		code = Status::i507;
	else if (error == EMFILE || error == ENFILE || error == ENOMEM)
		code = Status::i503;
	else
		code = Status::i500;
	errno = 0;
	return code;
}

CONNECTION_INL
(isize) init(int fd, VirtualServer* serverConfig) {
	clientFd = fd;
	cfg = serverConfig;
	readFd = -1;
	writeFd = -1;
	processId = -1;
	epollState = EPOLLIN;
	options = 0;
	contentType = Mime::OCTET_STREAM;
	bodySize = 0;
	chunkSize = 0;
	mode = Mode::PARSE_FIRST;
	recvBuffer.clear();
	req.clear();
	sendBuffer.clear();
	status.clear();
	startTime = Clock::time_elapsed();
	return 1;
}

CONNECTION_INL
(isize) end_connection() {
	clear();
	if (clientFd >= 0)
		close(clientFd);
	clientFd = -1;
	if (processId != -1)
		kill(processId, SIGKILL);
	return -1;
}

CONNECTION_INL
(void) clear() {
	if (mode == Mode::AUTOINDEX && directory != NULL) {
		closedir(directory);
		directory = NULL;
		readFd = -1;
	}
	if (readFd >= 0)
		close(readFd);
	if (writeFd >= 0)
		close(writeFd);
	readFd = -1;
	writeFd = -1;
}

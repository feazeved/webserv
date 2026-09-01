#pragma once
#include "Connection.hpp"

CONNECTION_INL
(isize) del_setup(Epoll &epoll) {
	Buffer64 pathBuffer = {};
	append_target_path(pathBuffer);

	struct stat st;
	if (stat(pathBuffer, &st) == -1)
		return flush_setup_close(epoll, s_get_status());
	if (S_ISDIR(st.st_mode))
		return flush_setup_close(epoll, Status::i403);	// Forbids deleting directories
	if (unlink(pathBuffer) == -1)
		return flush_setup_close(epoll, s_get_status());

	status = Status::i204;
	build_header();
	return write_to_client(epoll);
}

CONNECTION_INL
(isize) post_setup(Epoll &epoll) {
	Buffer64 pathBuffer = {};
	const Span uploadStore = req.location->get_upload_store();
	usize filename = req.target.size;
	while (filename > 0 && req.target.ptr[filename - 1] != '/')
		filename--;
	if (filename == req.target.size) {
		status = Status::i400;
		return -1;
	}
	pathBuffer.append(uploadStore);
	if (uploadStore.ptr[uploadStore.size - 1] != '/')
		pathBuffer.append("/");
	pathBuffer.append(req.target.ptr + filename, req.target.size - filename);
	*pathBuffer = 0;

	writeFd = open(pathBuffer, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NONBLOCK, 0644);
	if (writeFd == -1)
		return flush_setup_close(epoll, s_get_status());
	return download_file(epoll);
}

/*
	<html><head><title>Index of /download/</title></head><body>
	<h1>Index of /download/</h1><hr><pre><a href="../">../</a>
	<a href="nginx-0.1.0.tar.gz">nginx-0.1.0.tar.gz</a>                                 05-Oct-2004 15:39              220038
*/

#define HTTP_INDEX_HEADER "<html><head><title>Index of "
#define HTTP_INDEX_MIDDLE "</title></head><body><h1>Index of "
#define HTTP_INDEX_TAIL "</h1><hr><pre><a href=\"../\">../</a>"

CONNECTION_INL
(isize) get_directory_setup(Epoll &epoll, struct stat &st, Buffer64 &pathBuffer) {
	(void)st;
	const usize directoryLength = pathBuffer.writePos;
	const Span index = req.location->get_index();
	if (pathBuffer.writePos != 0 && pathBuffer.data[pathBuffer.writePos - 1] != '/')
		pathBuffer.append("/");
	pathBuffer.append(index.ptr + (index.ptr[0] == '/'), index.size - (index.ptr[0] == '/'));
	*pathBuffer = 0;
	readFd = open(pathBuffer, O_RDONLY | O_CLOEXEC | O_NONBLOCK);
	if (readFd >= 0) {
		if (stat(pathBuffer, &st) == -1) {	// TODO: Use fstat
			close(readFd);
			readFd = -1;
			return flush_setup_close(epoll, s_get_status());
		}
		pathBuffer.readPos = 0;
		pathBuffer.scanPos = pathBuffer.writePos;
		contentType = (u8)pathBuffer.match_mime();
		status = Status::i200;
		bodySize = (usize)st.st_size;
		build_header();
		return upload_file(epoll);
	}
	pathBuffer.writePos = directoryLength;
	*pathBuffer = 0;
	if (req.location->autoindex == false)
		return flush_setup_close(epoll, Status::i403);
	directory = opendir(pathBuffer);
	if (directory == NULL) {
		status = s_get_status();
		return flush_setup_close(epoll, s_get_status());
	}
	status = Status::i200;
	contentType = Mime::HTML;
	mode = Mode::AUTOINDEX;
	build_header();
	sendBuffer.append(HTTP_INDEX_HEADER);
	sendBuffer.append(req.target);
	sendBuffer.append(HTTP_INDEX_MIDDLE);
	sendBuffer.append(req.target);
	sendBuffer.append(HTTP_INDEX_TAIL);
	return upload_directory(epoll);
}

CONNECTION_INL
(isize) get_setup(Epoll &epoll) {
	Buffer64 pathBuffer = {};
	append_target_path(pathBuffer);

	struct stat st;
	if (stat(pathBuffer, &st) == -1)
		return flush_setup_close(epoll, s_get_status());

	if (S_ISDIR(st.st_mode))
		return get_directory_setup(epoll, st, pathBuffer);
	readFd = open(pathBuffer, O_RDONLY | O_CLOEXEC | O_NONBLOCK);
	if (readFd == -1)
		return flush_setup_close(epoll, s_get_status());
	status = Status::i200;
	bodySize = (usize)st.st_size;
	build_header();
	return upload_file(epoll);
}

CONNECTION_INL
(isize) flush_setup(Epoll &epoll, Status::Code code) {
	ASSERT(status.is_set(), "Status was not set in close connection");
	clear();
	mode = Mode::FLUSH;
	return write_to_client(epoll);		// Keep the connection alive until header is flushed
}

CONNECTION_INL
(isize) flush_setup_close(Epoll &epoll, Status::Code code) {
	ASSERT(status.is_set(), "Status was not set in close connection");
	clear();
	options &= ~(u16)Options::KEEP_ALIVE;
	mode = Mode::FLUSH;
	build_error_header();
	return write_to_client(epoll);		// Keep the connection alive until header is flushed
}

CONNECTION_INL
(isize) setup(Epoll &epoll) {
	if (validate_header(epoll))
		return 0;
	if (options & Options::CHUNKED_LENGTH)
		bodySize = cfg->maxBodySize;

	startTime = Clock::time_elapsed();	// Resets the clock on a valid response header
	if (req.location->redirectStatus.is_valid()) {
		status = (Status::Code)req.location->redirectStatus.index;
		bodySize = 0;
		options &= ~(u16)Options::KEEP_ALIVE;
		mode = Mode::FLUSH;
		sendBuffer.clear();
		sendBuffer.append("HTTP/1.1 ");
		sendBuffer.append(status.status_str());
		sendBuffer.append("\r\nLocation: ");
		sendBuffer.append(req.location->get_redirect_target());
		sendBuffer.append("\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");
		return write_to_client(epoll);
	}
	mode = (options & Options::CGI) ? Mode::CGI : (Mode::e_http_mode)(options & 7);
	if (mode == Mode::GET)
		return get_setup(epoll);
	if (mode == Mode::POST)
		return post_setup(epoll);
	if (mode == Mode::CGI)
		return cgi_setup(epoll);
	return del_setup(epoll);
}

#pragma once
#include "Connection.hpp"

CONNECTION_INL
(isize) get_setup(Epoll &epoll) {
	Buffer64 pathBuffer = {};
	append_target_path(pathBuffer);

	struct stat st;
	if (stat(pathBuffer, &st) == -1)
		return flush_setup_close(epoll, s_get_status());
	if (S_ISDIR(st.st_mode))
		return get_directory_setup(epoll, pathBuffer);
	readFd = open(pathBuffer, O_RDONLY | O_CLOEXEC | O_NONBLOCK);
	if (readFd == -1)
		return flush_setup_close(epoll, s_get_status());
	bodySize = (usize)st.st_size;
	contentType = fn::match_mime(pathBuffer.get_span());
	build_header(Status::i200);
	return upload_file(epoll);
}

#define HTTP_INDEX_HEADER "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n<html><head><title>Index of "
#define HTTP_INDEX_MIDDLE "</title></head><body><h1>Index of "
#define HTTP_INDEX_TAIL "</h1><hr><pre><a href=\"../\">../</a>"

CONNECTION_INL
(isize) get_directory_setup(Epoll &epoll, Buffer64 &pathBuffer) {
	const usize directoryLength = pathBuffer.writePos;
	const Span index = req.location->get_index();
	if (pathBuffer.writePos != 0 && pathBuffer.data[pathBuffer.writePos - 1] != '/')
		pathBuffer.append("/");
	pathBuffer.append(index.ptr + (index.ptr[0] == '/'), index.size - (index.ptr[0] == '/'));
	*pathBuffer = 0;
	readFd = open(pathBuffer, O_RDONLY | O_CLOEXEC | O_NONBLOCK);
	if (readFd >= 0) {
		struct stat st;
		if (fstat(readFd, &st) == -1 || (usize)st.st_size >= MAX_FILE_SIZE || !S_ISREG(st.st_mode)) {
			close(readFd);
			readFd = -1;
			return flush_setup_close(epoll, s_get_status());
		}
		contentType = fn::match_mime(pathBuffer.get_span());
		bodySize = (usize)st.st_size;
		build_header(Status::i200);
		return upload_file(epoll);
	}
	pathBuffer.writePos = directoryLength;
	*pathBuffer = 0;
	if (req.location->autoindex == false)
		return flush_setup_close(epoll, Status::i403);
	const usize targetSize = fn::html_encoded_size(req.target.ptr, req.target.size);
	const usize fixedSize = sizeof(HTTP_INDEX_HEADER) + sizeof(HTTP_INDEX_MIDDLE) + sizeof(HTTP_INDEX_TAIL) - 3;
	const usize headerSize = fixedSize + targetSize * 2;
	if (headerSize > sendBuffer.capacity())
		return flush_setup_close(epoll, Status::i414);
	directory = opendir(pathBuffer);
	if (directory == NULL) 
		return flush_setup_close(epoll, s_get_status());
	status = Status::i200;
	contentType = Mime::HTML;
	mode = Mode::AUTOINDEX;
	options &= ~(u16)Options::KEEP_ALIVE;
	sendBuffer.append(HTTP_INDEX_HEADER);
	sendBuffer.append_html(req.target.ptr, req.target.size);
	sendBuffer.append(HTTP_INDEX_MIDDLE);
	sendBuffer.append_html(req.target.ptr, req.target.size);
	sendBuffer.append(HTTP_INDEX_TAIL);
	return upload_directory(epoll);
}

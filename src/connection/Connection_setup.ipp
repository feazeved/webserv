#pragma once
#include "Connection.hpp"

CONNECTION_INL
(isize) del_setup(Epoll &epoll) {
	Buffer64 pathBuffer = {};
	pathBuffer.append(req.location->get_root());
	pathBuffer.append(req.target);

	struct stat st;
	if (stat(pathBuffer, &st) == -1)
		return s_get_status(status);
	if (S_ISDIR(st.st_mode))
		return s_get_status(status);	// Forbids deleting directories
	if (unlink(pathBuffer) == -1)
		return s_get_status(status);

	status = Status::i204;
	build_header();
	return write_to_client(epoll);
}

CONNECTION_INL
(isize) post_setup(Epoll &epoll) {
	Buffer64 pathBuffer = {};
	pathBuffer.append(req.location->get_root());
	pathBuffer.append(req.location->get_upload_store());

	writeFd = open(pathBuffer, O_WRONLY | O_CREAT | O_EXCL, 0644);
	if (writeFd == -1) {
		mode = Mode::CLOSE;
		return s_get_status(status);
	}
	return download_file(epoll);
}

/*
	<html><head><title>Index of /download/</title></head><body>
	<h1>Index of /download/</h1><hr><pre><a href="../">../</a>
	<a href="nginx-0.1.0.tar.gz">nginx-0.1.0.tar.gz</a>                                 05-Oct-2004 15:39              220038
*/

#define HTTP_INDEX_HEADER "<html><head><title>Index of /download/</title></head><body><h1>Index of "
#define HTTP_INDEX_MIDDLE "/</title></head><body><h1>Index of "
#define HTTP_INDEX_TAIL "/</h1><hr><pre><a href=\"../\">../</a>"

CONNECTION_INL
(isize) get_directory(Epoll &epoll, struct stat &st, Buffer64 &pathBuffer) {
	pathBuffer.append("/index.html");
	readFd = open(pathBuffer, O_RDONLY);
	if (readFd == -1 && req.location->autoindex == false)
		return s_get_status(status);
	if (readFd == -1) {
		pathBuffer.writePos -= sizeof("/index.html");	// TODO: add overwrite function
		*pathBuffer = 0;
		directory = opendir(pathBuffer);
		if (directory == NULL)
			return s_get_status(status);
	}
	status = Status::i200;
	bodySize = (usize)st.st_size;
	build_header();
	sendBuffer.append(HTTP_INDEX_HEADER);
	sendBuffer.append(req.target);			// TODO: Actually might need to be the last /
	sendBuffer.append(HTTP_INDEX_MIDDLE);
	sendBuffer.append(req.target);
	sendBuffer.append(HTTP_INDEX_TAIL);
	return upload_directory(epoll);
}

CONNECTION_INL
(isize) get_setup(Epoll &epoll) {
	Buffer64 pathBuffer = {};
	pathBuffer.append(req.location->get_root());
	pathBuffer.append(req.target);

	struct stat st;
	if (stat(pathBuffer, &st) == -1)
		return s_get_status(status);

	if (S_ISDIR(st.st_mode))
		return get_directory(epoll, st, pathBuffer);
	readFd = open(pathBuffer, O_RDONLY);
	if (readFd == -1)
		return s_get_status(status);
	status = Status::i200;
	bodySize = (usize)st.st_size;
	build_header();
	return upload_file(epoll);
}

CONNECTION_INL
(isize) setup(Epoll &epoll) {
	if (options & Options::CHUNKED_LENGTH)
		bodySize = cfg->maxBodySize;

	mode = (Mode::e_http_mode)(options & 0x0F);
	if (mode == Mode::GET)
		return get_setup(epoll);
	if (mode == Mode::POST)
		return post_setup(epoll);
	if (mode == Mode::CGI)
		return cgi_setup(epoll);
	return del_setup(epoll);
}

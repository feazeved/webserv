#pragma once
#include "Connection.hpp"

// <a href="filename[256]">filename[64]</a>    02-Dec-2004 18:46    241476
// The filename (256) + filename display (64) + 17 date + 19 for digits + 6 tabs + 16 html stuff

#define HTTP_INDEX_PERMISSION "<a href=\"\">--- Privileged access ---</a>\n"
#define HTTP_INDEX_HEADER "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n<html><head><title>Index of "
#define HTTP_INDEX_MIDDLE "</title></head><body><h1>Index of "
#define HTTP_INDEX_TAIL "</h1><hr><pre><a href=\"../\">../</a>"

static inline
usize s_append_entry(HTTP_Buffer &src, DIR* directory, struct dirent *dirEntry) {
	Span entry = {dirEntry->d_name, STRLEN(dirEntry->d_name)};

	struct stat st;

	if (STRCMP(entry.ptr, ".\0") == 0 || STRCMP(entry.ptr, "..\0") == 0)
		return 0;
	if (fstatat(dirfd(directory), dirEntry->d_name, &st, 0)) {
		src.append(HTTP_INDEX_PERMISSION);
		errno = 0;
		return sizeof(HTTP_INDEX_PERMISSION) - 1;
	}

	usize fileSize = S_ISDIR(st.st_mode) ? 0 : (usize) st.st_size;
	char buf[32];
	Clock::format_time(&st.st_mtim, buf);

	char* start = src.append("<a href=\"");
	// src.append(entry);
	src.append_url_component(entry.ptr, entry.size);
	src.append("\">");

	if (entry.size >= 64) {
		src.append_html(entry.ptr, 61);
		src.append("...");
	}
	else {
		src.append_html(entry.ptr, entry.size);
		src.memset(' ', 64 - entry.size);
	}
	src.append("</a>");
	src.memset('\t', 4);
	src.append_inline<17>(buf, 17);
	src.memset('\t', 2);
	src.append_digit10(fileSize);
	src.append("\n");
	return (usize)(src.wptr() - start);
}

// Finished state means everything is read to the send buffer and it only needs flushing of the send buffer
CONNECTION_INL
(isize) upload_directory(Epoll &epoll) {
	const usize bytesFree = sendBuffer.reserve(HTTP_DIRENT_MAX_SIZE);

	if (bytesFree < HTTP_DIRENT_MAX_SIZE)
		return write_to_client(epoll);	// Might need to flush the buffer

	const usize bytesMax = MIN(ATOMIC_IOSIZE, bytesFree - HTTP_DIRENT_MAX_SIZE);
	usize bytesTotal = 0;
	while (bytesTotal < bytesMax) {
		errno = 0;
		struct dirent* entry = readdir(directory);
		if (entry == NULL) {
			if (errno != 0)
				return -1;
			closedir(directory);
			directory = NULL;
			readFd = -1;
			sendBuffer.append("</pre></body></html>");
			return flush_setup(epoll, Status::i200);
		}
		bytesTotal += s_append_entry(sendBuffer, directory, entry);
	}
	return write_to_client(epoll);
}

/*
	<html><head><title>Index of /download/</title></head><body>
	<h1>Index of /download/</h1><hr><pre><a href="../">../</a>
	<a href="nginx-0.1.0.tar.gz">nginx-0.1.0.tar.gz</a>                                 05-Oct-2004 15:39              220038
*/

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
		if (fstat(readFd, &st) == -1) {
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

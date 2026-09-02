#pragma once
#include "Connection.hpp"

// <a href="filename[256]">filename[64]</a>    02-Dec-2004 18:46    241476
// The filename (256) + filename display (64) + 17 date + 19 for digits + 6 tabs + 16 html stuff

#define HTTP_INDEX_PERMISSION "<a href=\"\">--- Privileged access ---</a>"

static inline
void s_append_url_component(HTTP_Buffer &buffer, const char *ptr, usize length) {
	static const char hex[] = "0123456789ABCDEF";
	for (usize index = 0; index < length; index++) {
		const u8 value = (u8)ptr[index];
		if (IS_ALNUM(value) || value == '-' || value == '_' || value == '.' || value == '~')
			buffer.append((const char*)&ptr[index], 1);
		else {
			const char encoded[3] = {'%', hex[value >> 4], hex[value & 15]};
			buffer.append_inline<3>(encoded, 3);
		}
	}
}

static inline
void s_append_html(HTTP_Buffer &buffer, const char *ptr, usize length) {
	for (usize index = 0; index < length; index++) {
		switch (ptr[index]) {
			case '&': buffer.append("&amp;"); break;
			case '<': buffer.append("&lt;"); break;
			case '>': buffer.append("&gt;"); break;
			case '"': buffer.append("&quot;"); break;
			case '\'': buffer.append("&#39;"); break;
			default: buffer.append(ptr + index, 1); break;
		}
	}
}

static inline
usize s_append_entry(HTTP_Buffer &src, DIR* directory, struct dirent *dirEntry) {
	Span entry = {dirEntry->d_name, STRLEN(dirEntry->d_name)};

	struct stat st;

	if (STRCMP(entry.ptr, ".") == 0 || STRCMP(entry.ptr, "..") == 0)
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
	s_append_url_component(src, entry.ptr, entry.size);
	src.append("\">");

	if (entry.size >= 64) {
		// src.append_inline<61>(entry.ptr, 61);
		s_append_html(src, entry.ptr, 61);
		src.append("...");
	}
	else {
		// src.append(entry);
		s_append_html(src, entry.ptr, entry.size);
		src.memset(' ', 64 - entry.size);
	}
	src.memset('\t', 4);
	src.append_inline<17>(buf, 17);
	src.memset('\t', 2);
	src.append_digit10(fileSize);
	src.append("\n");
	return (usize)(src.wptr() - start);
}

// Finished state means everything is read to the send buffer and it only needs flushing of the send buffer
CONNECTION_INL
(isize) upload_file(Epoll &epoll) {
	isize bytesRead = sendBuffer.read(readFd, ATOMIC_IOSIZE);
	if (bytesRead == 0) {
		close(readFd);
		readFd = -1;
		return flush_setup(epoll, Status::i200);
	}
	else if (bytesRead == -1)
		return -1;
	return write_to_client(epoll);
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
			readFd = -1;
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
		contentType = fn::match_mime(pathBuffer.get_span());
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

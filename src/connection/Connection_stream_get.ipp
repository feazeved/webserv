#pragma once
#include "Connection.hpp"

// <a href="filename[256]">filename[64]</a>    02-Dec-2004 18:46    241476
// The filename (256) + filename display (64) + 17 date + 19 for digits + 6 tabs + 16 html stuff

#define HTTP_INDEX_PERMISSION "<a href=\"\">--- Privileged access ---</a>"

static inline
usize s_append_entry(HTTP_Buffer &src, struct dirent *dirEntry) {
	Span entry = {dirEntry->d_name, STRLEN(dirEntry->d_name)};
	struct stat st;
	if (stat(entry.ptr, &st) == -1) {
		if (errno != EACCES || st.st_size < 0)
			return SIZE_MAX;
		src.append(HTTP_INDEX_PERMISSION);
		errno = 0;
		return sizeof(HTTP_INDEX_PERMISSION);
	}
	usize fileSize = S_ISDIR(st.st_mode) ? 0 : (usize) st.st_size;
	char buf[32];
	Clock::format_time(&st.st_mtim, buf);

	char* start = src.append("<a href=\"");
	src.append(entry);
	src.append("\">");

	if (entry.size >= 64) {
		src.append_inline<61>(entry.ptr, 61);
		src.append("...");
	}
	else {
		src.append(entry);
		src.memset(' ', 64 - entry.size);
	}
	src.memset('\t', 4);
	src.append_inline<17>(buf, 17);
	src.memset('\t', 2);
	src.append_digit10(fileSize);
	return (usize)(src.wptr() - start);
}

// Maybe template this with a first run?
CONNECTION_INL
(isize) upload_file(Epoll &epoll) {
	isize bytesRead = sendBuffer.read(readFd, ATOMIC_IOSIZE);
	if (bytesRead == 0) {
		close(readFd);
		readFd = -1;
		bool keepAlive = !!(options & Options::KEEP_ALIVE);
		mode = keepAlive ? Mode::FLUSH : Mode::CLOSE;
	}
	else if (bytesRead == -1) {
		close(readFd);
		readFd = -1;
		mode = Mode::CLOSE;
		return close_connection(false);
	}
	return write_to_client(epoll);
}

CONNECTION_INL
(isize) upload_directory(Epoll &epoll) {
	errno = 0;
	struct dirent* entry = readdir(directory);
	const usize bytesFree = sendBuffer.reserve(HTTP_DIRENT_MAX_SIZE);

	if (bytesFree < HTTP_DIRENT_MAX_SIZE)
		return write_to_client(epoll);	// Might need to flush the buffer

	const usize bytesMax = MIN(ATOMIC_IOSIZE, bytesFree - HTTP_DIRENT_MAX_SIZE);
	usize bytesTotal = 0;
	while (bytesTotal < bytesMax) {
		entry = readdir(directory);
		if (entry == NULL) {
			closedir(directory);
			readFd = -1;
			if (errno != 0)
				close_connection(false);
			bool keepAlive = !!(options & Options::KEEP_ALIVE);
			mode = keepAlive ? Mode::FLUSH : Mode::CLOSE;
		}
		bytesTotal += s_append_entry(sendBuffer, entry);
	}
	return write_to_client(epoll);
}

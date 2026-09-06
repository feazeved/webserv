#pragma once
#include "Connection.hpp"

// <a href="filename[256]">filename[64]</a>    02-Dec-2004 18:46    241476
// The filename (256) + filename display (64) + 17 date + 19 for digits + 6 tabs + 16 html stuff

#define HTTP_INDEX_PERMISSION "<a href=\"\">--- Privileged access ---</a>\n"

static inline
usize s_append_entry(HTTP_Buffer &src, DIR* directory, struct dirent *dirEntry) {
	Span entry = {dirEntry->d_name, STRLEN(dirEntry->d_name)};

	struct stat st;
	if (LITCMP(entry.ptr, ".\0") == 0 || LITCMP(entry.ptr, "..\0") == 0)
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
			sendBuffer.append("</pre></body></html>");
			return flush_setup(epoll, Status::i200);
		}
		bytesTotal += s_append_entry(sendBuffer, directory, entry);
	}
	return write_to_client(epoll);
}

// Finished state means everything is read to the send buffer and it only needs flushing of the send buffer
CONNECTION_INL
(isize) upload_file(Epoll &epoll) {
	isize bytesRead = sendBuffer.read(readFd, MIN((usize)ATOMIC_IOSIZE, bodySize));
	if (bytesRead == -2)
		return write_to_client(epoll);
	if (bytesRead <= 0 && (bytesRead == -1 || bodySize != 0))
		return -1;
	bodySize -= (usize)bytesRead;
	if (bodySize == 0)
		return flush_setup(epoll, Status::i200);
	return write_to_client(epoll);
}

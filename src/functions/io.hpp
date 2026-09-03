#pragma once
// #include <unistd.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <netdb.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "core.hpp"
#include "Arena.hpp"
#include "Span.hpp"

namespace fn {

FN_ATTR(always_inline, pure, flatten) static
bool set_stream_mode(int fd) {
	bool result = fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
	result = result || fcntl(fd, F_SETFD, fcntl(fd, F_GETFD, 0) | FD_CLOEXEC);
	return result;
}

FN_ATTR(always_inline, pure) static
Span alloc_whole_file(Arena &arena, const char *filePath, int &fd, usize padSize = 32, usize minSize = 0, usize maxSize = UINT32_MAX) {
	struct stat st;
	Span result = {};

	if (stat(filePath, &st) == -1 || (usize)st.st_size < minSize || (usize)st.st_size >= maxSize)
		PERR_RETURN(result, "Error: Invalid file");

	fd = open(filePath, O_RDONLY);
	if (fd == -1)
		PERR_RETURN(result, "Error: Failed to open file");

	const usize fileSize = (usize)st.st_size;
	const u32 fileOffset = arena.alloc(fileSize, 1 + padSize);
	if (fileOffset == UINT32_MAX) {
		close(fd);
		PERR_RETURN(result, "Error: Out of memory");
	}
	result.ptr = (char*) arena.mptr(fileOffset);
	result.size = fileSize;
	return result;
}

FN_ATTR(always_inline, pure, flatten) static
bool read_whole_file(Arena &arena, const char *filePath, Span &file, usize padSize = 32, usize minSize = 0, usize maxSize = UINT32_MAX) {
	struct stat st;
	if (stat(filePath, &st) == -1 || (usize)st.st_size < minSize || (usize)st.st_size >= maxSize)
		PERR_RETURN(1, "Error: Invalid file");

	int fd = open(filePath, O_RDONLY);
	if (fd == -1)
		PERR_RETURN(1, "Error: Failed to open file");

	const usize fileSize = (usize)st.st_size;
	const u32 fileOffset = arena.alloc(fileSize, 1 + padSize);
	if (fileOffset == UINT32_MAX) {
		close(fd);
		PERR_RETURN(1, "Error: Out of memory");
	}

	u8* ptr = arena.mptr(fileOffset);
	usize curBytes = 0;
	while (curBytes < fileSize) {
		usize bytesRemaining = fileSize - curBytes;
		isize bytesRead = read(fd, ptr + curBytes, MIN(bytesRemaining, ATOMIC_IOSIZE));
		if (bytesRead <= 0) {
			close(fd);
			PERR_RETURN(1, "Error: Read failure");
		}
		curBytes += (usize) bytesRead;
	}
	close(fd);
	ptr[fileSize] = '\0';
	file.ptr = (char*)ptr;
	file.size = fileSize;
	return 0;
}

}
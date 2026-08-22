#pragma once
#include "core.hpp"
#include "sys/stat.h"
#include "VirtualServer.hpp"

namespace HTTP {
// Default error header
// <!doctype html><title>404</title><h1>404</h1>

// Considering this is an error, the connection will close. Which means you can override the buffer! hell yeah
// The only memory saving you really need to do is for loading error_pages

static inline
bool s_read_file(char *filePath, char *appendPtr, StringView &out) {

	MEMCPY(appendPtr, out.get(), out.length + 1);
	
	int fd = open(filePath, O_RDONLY);
	if (fd == -1)
		PERR_RETURN(1, "Error: Failed to open file");

	struct stat st;
	if (fstat(fd, &st) == -1 || st.st_size < 16 || (usize)st.st_size > MAX_FILE_SIZE - 127) {
		close(fd);
		PERR_RETURN(1, "Error: Invalid file");
	}

	usize fileSize = (usize) st.st_size;
	usize allocSize = ALIGN_UP(fileSize + 63, (usize)64);	// Pads with at least 64 bytes
	out.offset = Arena::alloc_index(allocSize);
	out.length = (u32) fileSize;

	u8* ptr = Arena::data + out.offset;
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
	return 0;
}


// TODO: Something like this will have to be present in the arena, since string views operate
// on arena
static const char* s_default_client_error_pages[32] = {
    "<!doctype html><title>400</title><h1>400 Bad Request</h1>",
    "<!doctype html><title>401</title><h1>401 Unauthorized</h1>",
    "<!doctype html><title>402</title><h1>402 Payment Required</h1>",
    "<!doctype html><title>403</title><h1>403 Forbidden</h1>",
    "<!doctype html><title>404</title><h1>404 Not Found</h1>",
    "<!doctype html><title>405</title><h1>405 Method Not Allowed</h1>", // ... continue here
};

static inline
void s_read_memory(usize error_index, StringView &out) {
	out.ptr = s_default_client_error_pages[error_index];
	out.length = STRLEN(out.ptr);
}

// TODO: assert that no individual path is longer than 4096
VIRTUALSERVER_INL
(bool) cache_error_pages() {
	static const usize errorSize = ARRAY_SIZE(clientErrors);
	char pathBuffer[16 * 1024];

	usize length[errorSize];
	/*
		TODO: for identical file names, load file only once, then have string views
		of that file instead of opening the same file multiple times
	*/

	MEMCPY(pathBuffer, serverRoot.get(), serverRoot.length);
	char *appendPtr = pathBuffer + serverRoot.length;

	for (usize i = 0; i < ARRAY_SIZE(clientErrors); i++) {
		if (clientErrors[i].length == 0)
			s_read_memory(i, clientErrors[i]);
		else if (s_read_file(pathBuffer, appendPtr, clientErrors[i]) == 1) {
			// Call big papa server and clean all listenFds
			return 1;
		}
	}

	for (usize i = 0; i < ARRAY_SIZE(serverErrors); i++) {
		StringView &ref = serverErrors[i];
		MEMCPY(appendPtr, ref.get(), ref.length + 1);
		if (s_read_file(pathBuffer, ref) == 1) {
			// Call big papa server and clean all listenFds
			return 1;
		}
	}
}

/*
	Normalize CGI Input here, so that the length is encoded in the strings
	I was considering making a locations class that compacts its members,
	and gives easy access to each locations member. Like, a method that returns
	a string view pair of the next CGI extension and interpreter
*/
VIRTUALSERVER_INL
(bool) process_cgi_block() {
	
}
}
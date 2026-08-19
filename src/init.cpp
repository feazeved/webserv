#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <new>
#include "core.hpp"
#include "HTTP.hpp"

static inline
char* s_read_whole_file(int fd, usize totalBytes) {
	usize allocSize = ALIGN_UP(totalBytes + 63, (usize)64);	// Pads with at least 64 bytes

	char *fileBuffer = new (std::nothrow) char[allocSize];
	if (fileBuffer == NULL) {
		close(fd);
		PERR_EXIT(1, "Error: Allocation failure");
	}

	usize curBytes = 0;
	while (curBytes < totalBytes) {
		usize bytesRemaining = totalBytes - curBytes;
		isize bytesRead = read(fd, fileBuffer + curBytes, MIN(bytesRemaining, ATOMIC_IOSIZE));
		if (bytesRead <= 0) {
			close(fd);
			delete[] fileBuffer;
			PERR_EXIT(1, "Error: Read failure");
		}
		curBytes += (usize) bytesRead;
	}
	fileBuffer[totalBytes] = 0;
	return fileBuffer;
}

static inline
isize s_find_server(const char* &str, const char *end) {
	while (IS_SPACE(*str))
		str++;
	if (MEMCMP_INLINE(str, "server") != 0)	// str is padded
		return (*str == 0 ? 0 : -1);
	str += 6;
	while (str < end && *str != '{')
		str++;
	str++;
	isize pdepth = 1;
	while (str < end && pdepth > 0) {
		pdepth += (*str == '{') - (*str == '}');
		str++;
	}
	return pdepth == 0 ? 1 : -1;
}

static inline
usize s_count_servers(const char *str, usize length) {
	const char *end = str + length;
	usize serverCount = 0;
	isize rvalue = 0;

	while (str < end) {
		rvalue = s_find_server(str, end);
		if (rvalue <= 0)
			break;
		serverCount++;
	}
	if (rvalue == -1)
		return SIZE_MAX;
	return serverCount;
}

void init(const char *str) {
	int fd = open(str, O_RDONLY);
	if (fd == -1)
		PERR_EXIT(1, "Error: Failed to open file");

	struct stat st;
	if (fstat(fd, &st) == -1 ) {	// TODO: might remove this failure path
		close(fd);
		PERR_EXIT(1, "Error: Failed to query file");
	}

	if (st.st_size < 16) {
		close(fd);
		PERR_EXIT(1, "Error: Invalid file");
	}
	usize totalBytes = (usize) st.st_size;
	char* fileBuffer = s_read_whole_file(fd, totalBytes);
	// usize numTokens = s_count_tokens(fileBuffer);
}

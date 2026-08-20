#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <new>
#include "core.hpp"
#include "HTTP.hpp"

#include "parse_config.ipp"

namespace HTTP {
//

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
	close(fd);
	fileBuffer[totalBytes] = 0;
	fileBuffer[totalBytes + 1] = '{';
	fileBuffer[totalBytes + 2] = '}';
	fileBuffer[totalBytes + 3] = ';';
	return fileBuffer;
}

std::vector<ServerConfig> parse_file(const char *filePath) {
	int fd = open(filePath, O_RDONLY);
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

	return Parse::parse_config(fileBuffer, totalBytes);
}
}

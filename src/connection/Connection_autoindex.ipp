#pragma once
#include "Connection.hpp"

struct s_entry {
	char name[128];
	u8 nameLength;
	usize fileSize;
	u8 date[32];
};

CONNECTION_INL
(isize) get_directory(struct stat &dirInfo, Buffer8 &pathBuffer) {
	DIR* directory = opendir(pathBuffer);
	struct dirent* entry;
	usize totalSize = 0;

	if (directory == NULL)
		return error_path();

	// for (usize i = 0; i < 256; i++) {
	// 	entry = readdir(directory);
	// 	if (entry == NULL) {
	// 		if (errno != 0) {
	// 			// return and close
	// 		}
	// 	}
	// 	dirBuffer[i].length = STRLEN(entry->d_name);
	// 	totalSize += dirBuffer[i].length + 1;
	// 	MEMCPY(dirBuffer[i].data, entry->d_name, dirBuffer[i].length);
	// }

	const usize maxSize = recvBuffer.capacity() - 64;

	// Check errno
}

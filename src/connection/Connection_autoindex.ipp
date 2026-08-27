#pragma once
#include "Connection.hpp"

struct s_entry {
	char name[128];
	u8 nameLength;
	usize fileSize;
	u8 date[32];
};

/*
	is this finally a good use for a linked list?? allocate different entry tables
	with a tagged union being the 2 MSB bits!
	Create a region (arena) where you have composition of the containers you created
	then everything fucking clicks! each container object contains a reference to the
	parent arena, no more of this fucking static bullshit!

	anyhow, linked lists were an interetsing idea to be contained within an arena
*/ 

// Connections could borrow buffers
// Remember the fairness mechanism. This could be streamed instead of accumulating
// Hold a tagged union for the FDs

CONNECTION_INL
(isize) get_directory(struct stat &dirInfo, Buffer8 &pathBuffer) {
	Matrix<256, 256> dirBuffer;				// If more than 8 kb of files, we cannot serve directly
	DIR* directory = opendir(pathBuffer);
	struct dirent* entry;
	usize totalSize = 0;

	if (directory == NULL)
		return error_path();

	for (usize i = 0; i < 256; i++) {
		entry = readdir(directory);
		if (entry == NULL) {
			if (errno != 0) {
				// return and close
			}
		}
		dirBuffer[i].length = STRLEN(entry->d_name);
		totalSize += dirBuffer[i].length + 1;
		MEMCPY(dirBuffer[i].data, entry->d_name, dirBuffer[i].length);
	}

	const usize maxSize = recvBuffer.capacity() - 64;

	// Check errno
}

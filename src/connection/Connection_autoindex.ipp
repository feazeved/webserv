#pragma once
#include "Connection.hpp"

struct s_entry {
	char name[128];
	u8 nameLength;
	usize fileSize;
	u8 date[32];
};

/*
<html>
<head><title>Index of /download/</title></head>
<body>
<h1>Index of /download/</h1><hr><pre><a href="../">../</a>
<a href="nginx-0.1.0.tar.gz">nginx-0.1.0.tar.gz</a>                                 05-Oct-2004 15:39              220038
<a href="nginx-0.1.1.tar.gz">nginx-0.1.1.tar.gz</a>                                 11-Oct-2004 15:06              224533
<a href="nginx-0.1.10.tar.gz">nginx-0.1.10.tar.gz</a>                                26-Nov-2004 09:35              239139
<a href="nginx-0.1.11.tar.gz">nginx-0.1.11.tar.gz</a>                                02-Dec-2004 18:46              241476
*/

// Each line has 16 bytes of HTML boilerplate
// The uri with file name appended

CONNECTION_INL
(isize) get_directory() {
	struct dirent* entry;
	usize totalSize = 0;

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

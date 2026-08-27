#pragma once
#include "Connection.hpp"

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

// <a href="filename[256]">filename[64]</a>    02-Dec-2004 18:46    241476
// Each line has 16 bytes of HTML boilerplate
// The filename (256) + filename display (64)

#define HTTP_INDEX_PERMISSION "<a href=\"\">Privileged access</a>"

static inline
void s_append_entry(HTTP_Buffer &src, struct dirent* entry) {
	const usize length = entry->d_reclen;

	struct stat st;
	if (stat(entry->d_name, &st) == -1) {
		if (errno == EACCES)
			src.append(HTTP_INDEX_PERMISSION);
		return;
	}

	char buf[32];
	Clock::format_time(u64 nanoseconds, char *buffer)
	src.append("<a href=\"");

}

CONNECTION_INL
(isize) get_autoindex() {
	
}
#pragma once
#include "Connection.hpp"

namespace HTTP {

static inline
void s_append_mime(Cursor &dst, u8 mimeIndex) {
	static const u8 mimeStrings[][32] = {"\x09" "text/html", "\x09" "text/html", 
	"\x08" "text/css", "\x10" "application/json", "\x16" "application/javascript",
	"\x09" "image/png", "\x0A" "image/jpeg", "\x0A" "image/jpeg", 
	"\x09" "image/gif", "\x0A" "text/plain", "\x18" "application/octet-stream"};

	const u8 *str = mimeStrings[mimeIndex];
	dst.append_inline<24>(str + 1, *str);
}

/*	Header is built in stack memory while parsing the header from client output
	When the header is built, it then appends part of the CGI body to tmp buffer
	up to how many bytes will fit in a single write */
CONNECTION_INL
(isize) build_cgi_header() {
	Buffer<2 * sizeof(clientInput)> tmpBuffer;
	Cursor &tmp = tmpBuffer.cursor;
	Cursor &src = clientOutput.cursor;
	Cursor &dst = clientInput.cursor;

	// tmpBuffer.cursor.index = 256;	// this isnt needed with header cache
	// tmpBuffer.cursor.size = 256;
	// tmpBuffer.cursor.start = 256;

	while (clientOutput.cursor.find_line_end() == 1) {
		if (request.parse_cgi_line(clientOutput.cursor, clientInput.cursor) == -1) {
			
		}
	}
}

CONNECTION_INL
(void) build_header() {
	Cursor &dst = clientInput.cursor;

	dst.append("HTTP/1.1 ");	// always use the buffer appends, cause it updates the cursors
	if (request.status.is_error()) {
		dst.append((const u8*) request.status.c_str(), request.status.size());
		dst.append("\r\n");
	}
	else {
		dst.append_inline<3>((const u8*) request.status.c_str(), 3);
		dst.append("OK\r\n");
	}

	dst.append("Content-Type: ");
	s_append_mime(dst, request.contentType);

	dst.append("\r\n\r\n");

	// clientInput.cursor.append("Content-Length: ");
	// s_append_content_length(clientInput, (usize)st.st_size);

	// client.append("HTTP/1.1 ");

	// if (bodySize != SIZE_MAX)
	// 	client.append("Transfer-Encoding: chunked\r\n");
	// else
	// {
	// 	client.append("Content-Length: ");
	// 	client.append(requestSize, false);	// Auto performs itoa
	// }
}
// Namespace HTTP
}

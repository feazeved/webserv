#pragma once
#include "Connection.hpp"

namespace HTTP {

static inline
void s_append_mime(Cursor &dst, u8 mimeIndex) {
	static const u8 mimeStrings[][32] = MIME_STRINGS;

	const u8 *str = mimeStrings[mimeIndex];
	dst.append_inline<24>(str + 1, *str);
}

/*	Header is built in stack memory while parsing the header from client output
	When the header is built, it then appends part of the CGI body to tmp buffer
	up to how many bytes will fit in a single write */
CONNECTION_INL
(isize) build_cgi_header() {
	Buffer<HTTP_BUFFERSIZE + 256> tmpBuffer;
	Cursor &tmp = tmpBuffer.cursor;
	Cursor &src = clientInput.cursor;

	tmp.writePtr += 256;
	tmp.readPtr += 256;

	while (src.find_line_end() == 1) {
		if (request.parse_cgi_line(src, tmp) == -1) {
			return error_path();
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

}
// Namespace HTTP
}

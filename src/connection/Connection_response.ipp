#pragma once
#include "Connection.hpp"

/*	Header is built in stack memory while parsing the header from client output
	When the header is built, it then appends part of the CGI body to tmp buffer
	up to how many bytes will fit in a single write */
// What do i get out of the header??
CONNECTION_INL
(isize) build_cgi_header() {
	Buffer16 tmpBuffer;

	tmpBuffer.writePos += 256;
	tmpBuffer.readPos += 256;

	while (sendBuffer.find_line_end() == 1) {
		if (parse_cgi_line(tmpBuffer) == -1) {
			return error_path();
		}
	}
}

CONNECTION_INL
(void) build_header() {
	Span str = status.status_str();

	sendBuffer.append("HTTP/1.1 ");
	if (status.is_error()) {
		sendBuffer.append(str.ptr, str.length);
		sendBuffer.append("\r\n");
		mode = Mode::CLOSE;
	}
	else {
		sendBuffer.append_inline<3>(str.ptr, 3);
		sendBuffer.append("OK\r\n");
		mode = Mode::FLUSH;
	}

	sendBuffer.append("Content-Type: ");
	sendBuffer.append_mime(contentType);
	sendBuffer.append("\r\n\r\n");
}

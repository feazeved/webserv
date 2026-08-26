#pragma once
#include "Connection.hpp"

/*	Header is built in stack memory while parsing the header from client output
	When the header is built, it then appends part of the CGI body to tmp buffer
	up to how many bytes will fit in a single write */
// What do i get out of the header??
CONNECTION_INL
(isize) build_cgi_header() {
	Buffer<HTTP_BUFFERSIZE> tmpBuffer;

	tmpBuffer.writePtr += 256;
	tmpBuffer.readPtr += 256;

	while (sendBuffer.find_line_end() == 1) {
		if (request.parse_cgi_line(sendBuffer, tmpBuffer) == -1) {
			return error_path();
		}
	}
}

CONNECTION_INL
(void) build_header() {
	Span str = request.status.status_str();

	sendBuffer.append("HTTP/1.1 ");
	if (request.status.is_error()) {
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
	sendBuffer.append_mime(request.contentType);
	sendBuffer.append("\r\n\r\n");
}

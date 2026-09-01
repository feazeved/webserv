#pragma once
#include "Connection.hpp"

CONNECTION_INL
(isize) build_cgi_header() {
	Buffer64 tmpBuffer = {};

	tmpBuffer.writePos += 256;
	tmpBuffer.readPos += 256;

	while (sendBuffer.find_line_end() == 1) {
		if (parse_cgi_line(tmpBuffer) == -1) {
			return -1;
		}
	}
}

CONNECTION_INL
(void) build_header() {
	Span str = status.status_str();

	// sendBuffer.clear();	// Should not be needed
	sendBuffer.append("HTTP/1.1 ");
	sendBuffer.append_inline<3>(str.ptr, 3);
	sendBuffer.append(" OK\r\nContent-Type: ");
	sendBuffer.append_mime(contentType);
	sendBuffer.append("\r\nContent-Length: ");
	sendBuffer.append_digit10(bodySize);
	if (options & Options::KEEP_ALIVE)
		sendBuffer.append("\r\nConnection: keep-alive\r\n\r\n");
	else
		sendBuffer.append("\r\nConnection: close\r\n\r\n");
}

// HTTP/1.1 404 Not Found
// Content-Type: text/plain
// Content-Length: 14
// Connection: close
// 404 Not Found

// This can be moved to buffer
CONNECTION_INL
(void) build_error_header() {
	Span statusStr = status.status_str();
	Span errorPage = status.error_page();

	options &= ~(u16)Options::KEEP_ALIVE;	// Should not be needed
	sendBuffer.clear();
	sendBuffer.append("HTTP/1.1 ");
	sendBuffer.append(statusStr);
	sendBuffer.append("\r\nContent-Type: text/html\r\nContent-Length: ");
	sendBuffer.append_digit10(errorPage.size);
	sendBuffer.append("\r\nConnection: close\r\n\r\n");
	sendBuffer.append(errorPage);
}

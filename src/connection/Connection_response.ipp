#pragma once
#include "Connection.hpp"

/*	Header is built in stack memory while parsing the header from client output
	When the header is built, it then appends part of the CGI body to tmp buffer
	up to how many bytes will fit in a single write */
// What do i get out of the header??
CONNECTION_INL
(isize) build_cgi_header() {
	Buffer64 tmpBuffer = {};

	tmpBuffer.writePos += 256;
	tmpBuffer.readPos += 256;

	while (sendBuffer.find_line_end() == 1) {
		if (parse_cgi_line(tmpBuffer) == -1) {
			return close_connection();
		}
	}
}

CONNECTION_INL
(void) build_header() {
	Span str = status.status_str();

	sendBuffer.append("HTTP/1.1 ");
	sendBuffer.append_inline<3>(str.ptr, 3);

	sendBuffer.append(" OK\r\nContent-Type: ");
	sendBuffer.append_mime(contentType);
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

	sendBuffer.append("HTTP/1.1 ");
	sendBuffer.append(statusStr.ptr, statusStr.size);
	sendBuffer.append("\r\nContent-Type: text/html\r\nContent-Length: ");
	sendBuffer.append_digit10(errorPage.size);
	sendBuffer.append("\r\nConnection: close\r\n");
	sendBuffer.append(errorPage);
	sendBuffer.append("\r\n\r\n");
}

#pragma once
#include "Connection.hpp"

CONNECTION_INL
(isize) build_cgi_header() {
	Buffer64 tmpBuffer = {};
	tmpBuffer.writePos += 256;
	tmpBuffer.readPos += 256;
	tmpBuffer.scanPos += 256;
	const usize headerEnd = sendBuffer.scanPos;
	sendBuffer.scanPos = sendBuffer.readPos;
	status = Status::i200;

	while (sendBuffer.readPos < headerEnd) {
		const usize lineLength = sendBuffer.find_line_end();
		if (lineLength == SIZE_MAX)
			return -1;
		if (lineLength == 0) {
			sendBuffer.readPos = sendBuffer.scanPos;
			break;
		}
		if (parse_cgi_line(tmpBuffer) == -1) {
			return -1;
		}
		sendBuffer.readPos = sendBuffer.scanPos;
	}
	if (sendBuffer.readPos != headerEnd)
		return -1;

	tmpBuffer.append("Connection: close\r\n\r\n");
	tmpBuffer.append(sendBuffer.rptr(), sendBuffer.size());
	tmpBuffer.prepend("\r\n");
	tmpBuffer.prepend(status.status_str());
	tmpBuffer.prepend("HTTP/1.1 ");
	if (tmpBuffer.size() > sendBuffer.capacity())
		return -1;
	sendBuffer.clear();
	sendBuffer.append(tmpBuffer.rptr(), tmpBuffer.size());
	options &= ~(u16)Options::KEEP_ALIVE;
	return 0;
}

CONNECTION_INL
(void) build_header(Status::Code code) {
	status = code;
	Span str = status.status_str();

	sendBuffer.clear();
	sendBuffer.append("HTTP/1.1 ");
	sendBuffer.append(str);
	sendBuffer.append("\r\nContent-Type: ");
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

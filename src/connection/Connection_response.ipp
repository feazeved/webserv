#pragma once
#include "Connection.hpp"

CONNECTION_INL
(Status::Code) build_cgi_header(Status::Code code) {
	Buffer64 tmpBuffer = {};
	tmpBuffer.writePos += 256;
	tmpBuffer.readPos += 256;
	tmpBuffer.scanPos += 256;

	const usize headerEnd = sendBuffer.scanPos;
	sendBuffer.scanPos = sendBuffer.readPos;

	while (sendBuffer.readPos < headerEnd) {
		const Span line = sendBuffer.find_line_end();
		if (line.ptr == NULL)
			return Status::i500;
		if (line.size == 0) {
			sendBuffer.readPos = sendBuffer.scanPos;
			break;
		}
		if (parse_cgi_line(tmpBuffer) != Status::ok)
			return Status::i500;
		sendBuffer.readPos = sendBuffer.scanPos;
	}
	if (sendBuffer.readPos != headerEnd)
		return Status::i500;

	Span statusStr = Status::s_status_str(code);
	
	tmpBuffer.append("Connection: close\r\n\r\n");
	tmpBuffer.append(sendBuffer.rptr(), sendBuffer.size());
	tmpBuffer.prepend("\r\n");
	tmpBuffer.prepend(statusStr);
	tmpBuffer.prepend("HTTP/1.1 ");
	if (tmpBuffer.size() > sendBuffer.capacity())
		return Status::i500;
	sendBuffer.clear();
	sendBuffer.append(tmpBuffer.rptr(), tmpBuffer.size());
	options &= ~(u16)Options::KEEP_ALIVE;
	return code;
}

CONNECTION_INL
(void) build_header(Status::Code code) {
	Span statusStr = Status::s_status_str(code);

	sendBuffer.clear();
	sendBuffer.append("HTTP/1.1 ");
	sendBuffer.append(statusStr);		// TODO: Make htis not depend on setting status
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

CONNECTION_INL
(void) build_error_header(Status::Code code) {
	Span statusStr = Status::s_status_str(code);
	Span errorPage = Status::s_error_page(code);

	options &= ~(u16)Options::KEEP_ALIVE;
	sendBuffer.clear();
	sendBuffer.append("HTTP/1.1 ");
	sendBuffer.append(statusStr);
	sendBuffer.append("\r\nContent-Type: text/html\r\nContent-Length: ");
	sendBuffer.append_digit10(errorPage.size);
	sendBuffer.append("\r\nConnection: close\r\n\r\n");
	sendBuffer.append(errorPage);
}

#pragma once
#include "Connection.hpp"

CONNECTION_INL
(isize) parse(Epoll &epoll) {
	usize lineLength;
	isize rvalue;

	if (read_from_client(epoll) < 0)
		return close_connection();
	while ((lineLength = recvBuffer.find_line_end()) != SIZE_MAX) {
		if (lineLength == 0) {
			recvBuffer.readPos = recvBuffer.scanPos;
			return validate_header(epoll);
		}
		if ((options & 7) == 0)	// Methods are not set
			rvalue = parse_first_line(lineLength);
		else
			rvalue = parse_line(lineLength);
		if (rvalue == -1)
			return close_connection();
	}

	if (recvBuffer.bytes_free() < recvBuffer.minReadSize) {
		status = Status::i431;	// Couldnt read from client, buffer is full
		return close_connection();
	}
	return 0;
}

CONNECTION_INL
(isize) dispatch(Epoll &epoll) {
	switch (mode) {
		case Mode::PARSE:	return parse(epoll); break;
		case Mode::GET:		return upload_file(epoll); break;
		case Mode::POST:	return download_file(epoll); break;
		case Mode::FLUSH:
		case Mode::CLOSE:	return write_to_client(epoll); break;
		case Mode::CGI:		return cgi_method(epoll); break;
		default: return -1;
	}
}

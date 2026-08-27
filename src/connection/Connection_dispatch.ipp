#pragma once
#include "Connection.hpp"

CONNECTION_INL
(isize) error_path() {
	if (readFd >= 0) {
		close(readFd);
		readFd = -1;
	}

	if (writeFd >= 0) {
		close(writeFd);
		writeFd = -1;
	}

	build_header();
	
	return -1;
}

CONNECTION_INL
(isize) parse(u32 events) {
	usize lineLength;
	isize rvalue;

	if (read_from_client(events) < 0)
		return error_path();
	while ((lineLength = recvBuffer.find_line_end()) != SIZE_MAX) {
		if (lineLength == 0) {
			recvBuffer.readPos = recvBuffer.scanPos;	// TODO: see if its not better to have buffer reset it
			return validate_header();
		}
		if ((options & 7) == 0)	// Methods are not set
			rvalue = parse_first_line(lineLength);
		else
			rvalue = parse_line(lineLength);
		if (rvalue == -1)
			return error_path();
	}

	if (recvBuffer.bytes_free() < recvBuffer.minReadSize) {
		status = Status::i431;	// Couldnt read from client, buffer is full
		return error_path();
	}
	return 0;
}

CONNECTION_INL
(isize) dispatch(u32 events) {

	switch (mode) {
		case Mode::PARSE:	return parse(events); break;
		case Mode::GET:		return upload_file(events); break;
		case Mode::POST:	return download_file(events); break;
		case Mode::FLUSH:
		case Mode::CLOSE:	return write_to_client(events); break;
		case Mode::CGI:		return cgi_method(); break;
		case Mode::SSE:		return sse_method(); break;
		default: return -1;
	}

}

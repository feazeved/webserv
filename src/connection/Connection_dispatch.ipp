#pragma once
#include "Connection.hpp"

CONNECTION_INL
(isize) parse(Epoll &epoll) {
	Span line;
	Status::Code code;

	if (read_from_client(epoll) < 0)
		return -1;
	while ((line = recvBuffer.find_line_end()) != NULL) {
		if (line.size == 0) {
			recvBuffer.readPos = recvBuffer.scanPos;
			return setup(epoll);
		}
		if ((options & 7) == 0)	// Methods are not set
			code = parse_first_line(line);
		else
			code = parse_line(line);
		if (code != Status::unset)
			return flush_setup_close(epoll, code);
	}
	if (recvBuffer.bytes_free() < recvBuffer.minReadSize)
		return flush_setup_close(epoll, Status::i431);
	return 0;
}

// TODO: Create a separate state for parse_first. Makes things less messy
/*
	A mode is the state that the connection is in. It's an exclusive variable, not a bitfield
	The connection starts in parse mode. When it is done parsing, it calls:

	Setup configures each mode and calls the execution of a method.
	When a method is done (GET, POST, CGI, AUTOINDEX) or when a non fatal error happens, it enters Flush mode

	Flush mode writes all the remaining bytes in sendBuffer, then either closes or goes back to parsing mode
	Whether it closes or not depends on if there was an error, or if it specified a keep-alive option
*/
CONNECTION_INL
(isize) dispatch(Epoll &epoll) {
	switch (mode) {
		case Mode::PARSE:		return parse(epoll); break;
		case Mode::GET:			return upload_file(epoll); break;
		case Mode::POST:		return download_file(epoll); break;
		case Mode::FLUSH:		return flush(epoll); break;
		case Mode::CGI:			return cgi(epoll); break;
		case Mode::AUTOINDEX:	return upload_directory(epoll); break;
		default: return -1;
	}
}

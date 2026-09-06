#pragma once
#include "Connection.hpp"

CONNECTION_INL
(isize) first_parse(Epoll &epoll) {
	isize bytesRead = read_from_client(epoll);
	if (bytesRead <= 0)
		return bytesRead;

	Span line = recvBuffer.find_line_end();
	if (line == NULL)
		return 0;

	Status::Code code = parse_first_line(line);
	if (code != Status::unset)
		return flush_setup_close(epoll, code);
	mode = Mode::PARSE;
	return parse(epoll);
}

CONNECTION_INL
(isize) parse(Epoll &epoll) {
	isize bytesRead = read_from_client(epoll);
	if (bytesRead <= 0)
		return bytesRead;

	Span line;
	while ((line = recvBuffer.find_line_end()) != NULL) {
		if (line.size == 0) {
			recvBuffer.readPos = recvBuffer.scanPos;
			return setup(epoll);
		}
		Status::Code code = parse_line(line);
		if (code != Status::unset)
			return flush_setup_close(epoll, code);
	}
	return 0;
}

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
		case Mode::FIRST_PARSE:		return first_parse(epoll); break;
		case Mode::PARSE:			return parse(epoll); break;
		case Mode::GET:				return upload_file(epoll); break;
		case Mode::POST:			return download_file(epoll); break;
		case Mode::POST_FIXED:		return download_file_fixed(epoll); break;
		case Mode::POST_CHUNKED:	return download_file_chunked(epoll); break;
		case Mode::FLUSH:			return flush(epoll); break;
		case Mode::CGI:				return cgi(epoll); break;
		case Mode::CGI_FIXED:		return cgi_fixed(epoll); break;
		case Mode::CGI_CHUNKED:		return cgi_chunked(epoll); break;
		case Mode::AUTOINDEX:		return upload_directory(epoll); break;
		default: return -1;
	}
}

#pragma once
#include "Connection.hpp"

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
		case Mode::PARSE_FIRST:		return parse_first(epoll);
		case Mode::PARSE:			return parse(epoll);
		case Mode::GET:				return upload_file(epoll);
		case Mode::POST_FIXED:		return download_file_fixed(epoll);
		case Mode::POST_CHUNKED:	return download_file_chunked(epoll);
		case Mode::FLUSH:			return flush(epoll);
		case Mode::CGI:				return cgi(epoll);
		case Mode::CGI_FIXED:		return cgi_fixed(epoll);
		case Mode::CGI_CHUNKED:		return cgi_chunked(epoll);
		case Mode::CGI_PARSED:		return cgi_parsed(epoll);
		case Mode::AUTOINDEX:		return upload_directory(epoll);
		default: return -1;
	}
}

CONNECTION_INL
(isize) parse_first(Epoll &epoll) {
	const isize result = read_from_client(epoll);
	if (result <= 0)
		return result;

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
	if (parseBuffer.writePos >= 16000)
		return flush_setup_close(epoll, Status::i431);

	const isize result = read_from_client(epoll);
	if (result <= 0)
		return result;

	Span line;
	while ((line = recvBuffer.find_line_end()) != NULL) {
		if (line.size == 0) {
			recvBuffer.readPos = recvBuffer.scanPos;
			return setup_dispatch(epoll);
		}
		Status::Code code = parse_line(line);
		if (code != Status::unset)
			return flush_setup_close(epoll, code);
	}
	return 0;
}

CONNECTION_INL
(isize) setup_dispatch(Epoll &epoll) {
	const bool isBodyMethod = options & Options::POST;
	const bool encodingSet = options & (Options::CHUNKED_LENGTH | Options::FIXED_LENGTH);

	if ((options & Options::HOST) == 0)
		return flush_setup_close(epoll, Status::i400);	// Host not set
	if (!isBodyMethod && encodingSet)
		return flush_setup_close(epoll, Status::i400);	// Encoding set for non-body methods
	if (isBodyMethod && !encodingSet)
		return flush_setup_close(epoll, Status::i411);	// Transfer encoding not set

	if (options & Options::CHUNKED_LENGTH)
		bodySize = cfg->maxBodySize;

	startTime = Clock::time_elapsed();	// Resets the clock on a valid response header
	sendBuffer.clear();
	if (req.location->redirectStatus.is_valid())
		return redirect_setup(epoll, (Status::Code)req.location->redirectStatus.index);
	if (options & Options::CGI && !(options & Options::POST))
		mode = Mode::CGI;
	else if (options & Options::CGI && (options & Options::POST))
		mode = (options & Options::FIXED_LENGTH) ? Mode::CGI_FIXED : Mode::CGI_CHUNKED;
	else if (options & Options::GET)
		mode = Mode::GET;
	else if (options & Options::POST)
		mode = (options & Options::FIXED_LENGTH) ? Mode::POST_FIXED : Mode::POST_CHUNKED;
	if (epoll.modify(clientFd, EPOLLIN | EPOLLOUT, epollState))
		return -1;
	if (mode == Mode::POST_FIXED || mode == Mode::POST_CHUNKED)
		return post_setup(epoll);
	if (mode == Mode::CGI || mode == Mode::CGI_FIXED || mode == Mode::CGI_CHUNKED)
		return cgi_setup(epoll);
	if (mode == Mode::GET)
		return get_setup(epoll);
	return del_setup(epoll);
}

#pragma once
#include "Connection.hpp"

CONNECTION_INL
(isize) del_setup(Epoll &epoll) {
	Buffer64 pathBuffer = {};
	append_target_path(pathBuffer);

	struct stat st;
	if (stat(pathBuffer, &st) == -1)
		return flush_setup_close(epoll, s_get_status());
	if (S_ISDIR(st.st_mode))
		return flush_setup_close(epoll, Status::i403);	// Forbids deleting directories
	if (unlink(pathBuffer) == -1)
		return flush_setup_close(epoll, s_get_status());
	build_header(Status::i204);
	return flush_setup(epoll, Status::i204);
}

CONNECTION_INL
(isize) parse_setup(Epoll &epoll) {
	options = 0;
	contentType = Mime::OCTET_STREAM;
	bodySize = 0;
	chunkSize = SIZE_MAX;
	mode = Mode::PARSE;
	req.clear();
	sendBuffer.clear();
	status.clear();
	startTime = Clock::time_elapsed();
	if (epoll.modify(clientFd, EPOLLIN, epollState))
		return -1;
	return parse(epoll);		// Keep the connection alive until header is flushed
}

CONNECTION_INL
(isize) flush_setup(Epoll &epoll, Status::Code code) {
	status = code;
	clear();
	mode = Mode::FLUSH;
	isize bytesWritten = write_to_client(epoll);
	if (sendBuffer.size() > 0 && epoll.modify(clientFd, EPOLLOUT, epollState))
		return -1;	// TODO: See if i can't just stream the output then close
	return bytesWritten;
}

CONNECTION_INL
(isize) flush_setup_close(Epoll &epoll, Status::Code code) {
	status = code;
	clear();
	options &= ~(u16)Options::KEEP_ALIVE;
	mode = Mode::FLUSH;
	build_error_header(code);
	if (epoll.modify(clientFd, EPOLLOUT, epollState))
		return -1;
	return write_to_client(epoll);
}

CONNECTION_INL
(isize) redirect_setup(Epoll &epoll, Status::Code code) {
	status = code;
	bodySize = 0;
	options &= ~(u16)Options::KEEP_ALIVE;
	mode = Mode::FLUSH;
	sendBuffer.append("HTTP/1.1 ");
	sendBuffer.append(status.status_str());
	sendBuffer.append("\r\nLocation: ");
	sendBuffer.append(req.location->get_redirect_target());
	sendBuffer.append("\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");
	if (epoll.modify(clientFd, EPOLLOUT, epollState))
		return -1;
	return write_to_client(epoll);
}

CONNECTION_INL
(isize) setup(Epoll &epoll) {
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
	mode = (options & Options::CGI) ? Mode::CGI : (Mode::e_http_mode)(options & 7);
	if (epoll.modify(clientFd, EPOLLIN | EPOLLOUT, epollState))
		return -1;
	if (mode == Mode::POST)
		return post_setup(epoll);
	if (mode == Mode::CGI)
		return cgi_setup(epoll);
	if (mode == Mode::GET)
		return get_setup(epoll);
	return del_setup(epoll);
}

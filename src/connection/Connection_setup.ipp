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

	status = Status::i204;
	build_header();
	return write_to_client(epoll);
}

CONNECTION_INL
(isize) setup(Epoll &epoll) {
	const bool isBodyMethod = options & Options::POST;
	const bool encodingSet = options & (Options::CHUNKED_LENGTH | Options::FIXED_LENGTH);

	if (status.is_set())
		return !flush_setup_close(epoll, (Status::Code) status.index);	// An error caused early interruption
	if ((options & 0xF) == 0)
		return !flush_setup_close(epoll, Status::i400);	// TODO: Method not set, should be impossible. Remove in future
	if ((options & Options::HOST) == 0)
		return !flush_setup_close(epoll, Status::i400);	// Host not set
	if (!isBodyMethod && encodingSet)
		return !flush_setup_close(epoll, Status::i400);	// Encoding set for non-body methods
	if (isBodyMethod && !encodingSet)
		return !flush_setup_close(epoll, Status::i411);	// Transfer encoding not set

	if (options & Options::CHUNKED_LENGTH)
		bodySize = cfg->maxBodySize;

	startTime = Clock::time_elapsed();	// Resets the clock on a valid response header
	if (req.location->redirectStatus.is_valid()) {
		status = (Status::Code)req.location->redirectStatus.index;
		bodySize = 0;
		options &= ~(u16)Options::KEEP_ALIVE;
		mode = Mode::FLUSH;
		sendBuffer.clear();
		sendBuffer.append("HTTP/1.1 ");
		sendBuffer.append(status.status_str());
		sendBuffer.append("\r\nLocation: ");
		sendBuffer.append(req.location->get_redirect_target());
		sendBuffer.append("\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");
		return write_to_client(epoll);
	}
	mode = (options & Options::CGI) ? Mode::CGI : (Mode::e_http_mode)(options & 7);
	if (mode == Mode::POST)
		return post_setup(epoll);
	if (mode == Mode::CGI)
		return cgi_setup(epoll);
	if (epoll.is_readable())
		epoll.clr_read(clientFd);
	if (mode == Mode::GET)
		return get_setup(epoll);
	return del_setup(epoll);
}

#pragma once
#include "Connection.hpp"

CONNECTION_INL
(isize) del_setup() {
	Buffer16 pathBuffer;
	pathBuffer.append(req.location->get_root());
	pathBuffer.append(req.target);
	pathBuffer.append("\0");

	struct stat st;
	if (stat(pathBuffer, &st) == -1)
		return s_get_status(status);
	if (S_ISDIR(st.st_mode))
		return s_get_status(status);	// Forbids deleting directories
	if (unlink(pathBuffer) == -1)
		return s_get_status(status);

	status = Status::i204;
	build_header();
	return 0;
}

CONNECTION_INL
(isize) post_setup() {
	Buffer16 pathBuffer;
	pathBuffer.append(req.location->get_root());
	pathBuffer.append(req.location->get_upload_store());
	pathBuffer.append("\0");

	writeFd = open(pathBuffer, O_WRONLY | O_CREAT | O_EXCL, 0644);
	if (writeFd == -1) {
		mode = Mode::CLOSE;
		return s_get_status(status);
	}
	return 0;
}

CONNECTION_INL
(isize) setup() {
	if (options & Options::CHUNKED_LENGTH)
		bodySize = cfg->maxBodySize;

	mode = (Mode::e_http_mode)(options & 0x0F);
	if (mode == Mode::GET)
		return get_setup();
	if (mode == Mode::POST)
		return post_setup();
	if (mode == Mode::CGI)
		return cgi_setup();
	return del_setup();		// TODO: All setup calls should call dispatch again
}

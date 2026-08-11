#pragma once
#include "Connection.hpp"

namespace HTTP {

CONNECTION_INL
(isize) error_path() {
		// Status should be set prior to entering error path
		// buildHeader();
		// Close the connection
		// Clean files
		// Reset state
}

CONNECTION_INL
(isize) configure() {
	const bool isBodyMethod = request.mode & (Mode::POST | Mode::CGI);
	const bool encodingSet = !(request.options & (Options::CHUNKED_LENGTH | Options::FIXED_LENGTH));

	if (request.status.is_set())
		return -1;	// An error caused early interruption

	if ((request.mode & 0xF) == 0)
		return -1;	// TODO: Method not set, should be impossible. Remove in future

	if ((request.mode & Options::HOST) == 0)
		return -1;	// Host not set

	if (isBodyMethod && !encodingSet)
		return -1;	// Transfer encoding not set
	if (!isBodyMethod && encodingSet)
		return -1;	// Encoding set for non-body methods
	request.bodySize = (request.bodySize == SIZE_MAX) ? SIZE_MAX : cfg->maxBodySize;
	
	if (request.mode & Mode::CGI)
		return cgi_first_run();
	else if (request.mode & (Mode::GET | Mode::DELETE))
		return get_first_run();
	else
		return post_first_run();
}

static inline
void s_append_mime(Cursor &dst, u8 mimeIndex) {
	static const u8 mimeStrings[][32] = {"\x09" "text/html", "\x09" "text/html", 
	"\x08" "text/css", "\x10" "application/json", "\x16" "application/javascript",
	"\x09" "image/png", "\x0A" "image/jpeg", "\x0A" "image/jpeg", 
	"\x09" "image/gif", "\x0A" "text/plain", "\x18" "application/octet-stream"};

	const u8 *str = mimeStrings[mimeIndex];
	dst.append_inline<24>(str + 1, *str);	// ignore warning, clang is being dumb
}

CONNECTION_INL
(void) build_header() {
	Cursor &dst = clientInput.cursor;

	dst.append("HTTP/1.1 ");	// always use the buffer appends, cause it updates the cursors
	if (request.status.is_error()) {
		dst.append((const u8*) request.status.c_str(), request.status.size());
		dst.append("\r\n");
	}
	else {
		dst.append_inline<3>((const u8*) request.status.c_str(), 3);
		dst.append("OK\r\n");
	}

	dst.append("Content-Type: ");
	s_append_mime(dst, request.contentType);

	dst.append("\r\n\r\n");

	// clientInput.cursor.append("Content-Length: ");
	// s_append_content_length(clientInput, (usize)st.st_size);

	// client.append("HTTP/1.1 ");

	// if (bodySize != SIZE_MAX)
	// 	client.append("Transfer-Encoding: chunked\r\n");
	// else
	// {
	// 	client.append("Content-Length: ");
	// 	client.append(requestSize, false);	// Auto performs itoa
	// }
}
}

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
		return error_path();	// An error caused early interruption

	if ((request.mode & 0xF) == 0)
		return error_path();	// TODO: Method not set, should be impossible. Remove in future

	if ((request.mode & Options::HOST) == 0)
		return error_path();	// Host not set

	if (isBodyMethod && !encodingSet)
		return error_path();	// Transfer encoding not set
	if (!isBodyMethod && encodingSet)
		return error_path();	// Encoding set for non-body methods
	request.bodySize = (request.bodySize == SIZE_MAX) ? SIZE_MAX : cfg->maxBodySize;
	
	if (request.mode & Mode::CGI)
		return cgi_first_run();
	else if (request.mode & (Mode::GET | Mode::DELETE))
		return get_first_run();
	else
		return post_first_run();
}

template <usize bufferSize>
static inline
isize s_append_mime(Buffer<bufferSize> &src, u8 mimeIndex) {
	static const char mimeStrings[][32] = {"\x09" "text/html", "\x09" "text/html", 
	"\x08" "text/css", "\x10" "application/json", "\x16" "application/javascript",
	"\x09" "image/png", "\x0A" "image/jpeg", "\x0A" "image/jpeg", 
	"\x09" "image/gif", "\x0A" "text/plain", "\x18" "application/octet-stream"};

	src.append_inline<24>(mimeStrings[mimeIndex] + 1, mimeStrings[mimeIndex]);	// ignore warning, clang is being dumb
}

CONNECTION_INL
(void) build_header() {

	clientInput.append("HTTP/1.1 ");	// always use the buffer appends, cause it updates the cursors
	if (request.status.is_error()) {
		clientInput.append(request.status.c_str(), request.status.size());
		clientInput.append("\r\n");
	}
	else {
		clientInput.append_inline(request.status.c_str(), 3);
		clientInput.append("OK\r\n");
	}

	clientInput.append("Content-Type: ");
	s_append_mime(clientInput, request.contentType);

	clientInput.append("\r\n\r\n");

	// clientInput.append("Content-Length: ");
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

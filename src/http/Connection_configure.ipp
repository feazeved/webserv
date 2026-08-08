#pragma once

#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include "core.hpp"
#include "http/Connection.hpp"

namespace HTTP {

template <usize bufferSize> inline
isize Connection<bufferSize>::error_path() {
		// Status should be set prior to entering error path
		// buildHeader();
		// Close the connection
		// Clean files
		// Reset state
}

// template <usize bufferSize> static inline
// void	s_append_cstr(Buffer<bufferSize>& buf, const char* str) {
// 	buf.append((const u8*)str, (usize)strlen(str));
// }

// template <usize bufferSize> static inline
// void	s_append_status_line(Buffer<bufferSize>& buf, u16 code, const char* reason) {
// 	char	line[16];
// 	i32		len = snprintf(line, sizeof(line), "HTTP/1.1 %u ", (unsigned)code);
// 	buf.append((const u8*)line, (usize)len);
// 	s_append_cstr(buf, reason);
// 	s_append_cstr(buf, "\r\n");
// }

// template <usize bufferSize> static inline
// void	s_append_content_length(Buffer<bufferSize>& buf, usize value) {
// 	char digits[24];
// 	int len = snprintf(digits, sizeof(digits), "%zu", value);
// 	buf.append((const u8*)digits, (usize)len);
// }

template <usize bufferSize> inline
isize Connection<bufferSize>::configure() {
	const bool isBodyMethod = type & (Attributes::POST | Attributes::CGI);
	const bool encodingSet = !(type & Attributes::CHUNKED) && bodySize == SIZE_MAX;

	if (status.is_set())
		return error_path();	// An error caused early interruption

	if ((type & 0xF) == 0)
		return error_path();	// TODO: Method not set, should be impossible. Remove in future

	if ((type & Attributes::HOST) == 0)
		return error_path();	// Host not set

	if (isBodyMethod && !encodingSet)
		return error_path();	// Transfer encoding not set
	if (!isBodyMethod && encodingSet)
		return error_path();	// Encoding set for non-body methods
	bodySize = (bodySize == SIZE_MAX) ? SIZE_MAX : cfg->maxBodySize;
	
	if (type & Attributes::CGI)
		return cgi_first_run();
	else if (type & (Attributes::GET | Attributes::DELETE))
		return get_first_run();
	else
		return post_first_run();
}

template <usize bufferSize> inline
void Connection<bufferSize>::build_header() {

	clientInput.append("HTTP/1.1 ");	// always use the buffer appends, cause it updates the cursors
	if (status.is_error()) {
		clientInput.append(status.c_str(), status.size());
		clientInput.append("\r\n");
	}
	else {
		clientInput.appendInline(status.c_str(), 3);
		clientInput.append("OK\r\n");
	}

	clientInput.append("Content-Type: ");
	// clientInput.append(s_get_mime_type(fullpath));	// lets have this already parsed, only print

	clientInput.append("\r\n\r\n");

	// s_append_cstr(clientInput, "Content-Type: ");
	// s_append_cstr(clientInput, s_get_mime_type(fullpath));
	// s_append_cstr(clientInput, "\r\n");
	// clientInput.append("Content-Length: ");
	// s_append_content_length(clientInput, (usize)st.st_size);
	// s_append_cstr(clientInput, "\r\n");
	// s_append_cstr(clientInput, "\r\n");

	// client.append("HTTP/1.1 ");

	// if (bodySize != SIZE_MAX)
	// 	client.append("Transfer-Encoding: chunked\r\n");
	// else
	// {
	// 	client.append("Content-Length: ");
	// 	client.append(requestSize, false);	// Auto performs itoa
	// }

	// Other lines here
	// Location
	// Content Type
	// Content Encoding?

	// client.append("\r\n");
	// if (isBad(status)) {
	// 	client.append(client.data + 9, statusEnd - 9);
	// 	return;
	// }
}
}

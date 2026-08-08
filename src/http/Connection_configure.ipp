#pragma once

#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include "core.hpp"
#include "http/Connection.hpp"

namespace HTTP {

template <usize bufferSize> inline
isize Connection<bufferSize>::error_path() {
		// buildHeader();
		// Close the connection
		// Clean files
		// Reset state
}

template <usize bufferSize> inline
isize Connection<bufferSize>::configure() {
	const bool isBodyMethod = type & (Attributes::POST | Attributes::CGI);
	const bool encodingSet = !(type & Attributes::CHUNKED) && bodySize == SIZE_MAX;

	if (status != 0)
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
void Connection<bufferSize>::buildHeader() {
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

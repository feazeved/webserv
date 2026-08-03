#pragma once

#include <unistd.h>
#include <sys/epoll.h>
#include "core.hpp"
#include "http/Request.hpp"

namespace HTTP {
// 
template <usize bufferSize> inline
isize Request<bufferSize>::configure() {
	static const u8 transferCheck = HTTP::Attributes::CHUNKED | HTTP::Attributes::POST;
	// Error checking
	if (status != 0) {	// An error caused early interruption
		// buildHeader();
		// Close the connection
	}

	if ((type & HTTP::Attributes::HOST) == 0) {	// Host wasnt set

	}

}

template <usize bufferSize> inline
void Request<bufferSize>::buildHeader() {
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

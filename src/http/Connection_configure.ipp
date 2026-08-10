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
	const bool isBodyMethod = request.type & (Attributes::POST | Attributes::CGI);
	const bool encodingSet = !(request.type & Attributes::CHUNKED) && request.bodySize == SIZE_MAX;

	if (request.status.is_set())
		return error_path();	// An error caused early interruption

	if ((request.type & 0xF) == 0)
		return error_path();	// TODO: Method not set, should be impossible. Remove in future

	if ((request.type & Attributes::HOST) == 0)
		return error_path();	// Host not set

	if (isBodyMethod && !encodingSet)
		return error_path();	// Transfer encoding not set
	if (!isBodyMethod && encodingSet)
		return error_path();	// Encoding set for non-body methods
	request.bodySize = (request.bodySize == SIZE_MAX) ? SIZE_MAX : cfg->maxBodySize;
	
	if (request.type & Attributes::CGI)
		return cgi_first_run();
	else if (request.type & (Attributes::GET | Attributes::DELETE))
		return get_first_run();
	else
		return post_first_run();
}

CONNECTION_INL
(void) build_header() {

	clientInput.append("HTTP/1.1 ");	// always use the buffer appends, cause it updates the cursors
	if (request.status.is_error()) {
		clientInput.append(request.status.c_str(), request.status.size());
		clientInput.append("\r\n");
	}
	else {
		clientInput.appendInline(request.status.c_str(), 3);
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

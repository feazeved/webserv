#pragma once
#include "Request.hpp"
#include "core.hpp"
#include "http/Request_helpers.hpp"

bool isBad(u8 test);

inline void HTTP::Request::buildHeader() {
	const u32 statusEnd = output.writeOffset - 2;

	if (requestSize != SIZE_MAX)
		output.append("Transfer-Encoding: chunked\r\n");
	else
	{
		output.append("Content-Length: ");
		output.append(requestSize, false);	// Auto performs itoa
	}

	// Other lines here
	// Location
	// Content Type
	// Content Encoding?

	output.append("\r\n");
	if (isBad(status)) {
		output.append(output.data + 9, statusEnd - 9);
		return;
	}
}

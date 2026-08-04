#pragma once

#include <unistd.h>

#include "core.hpp"
#include "http/Request.hpp"
#include "http/Parser.hpp"

namespace HTTP {

// Connection will loop until either there is an unrecoverable error, or a syscall was made
// The idea is that all information will be processed, and the limiting factor is the influx of new information
// The new information goes both ways, input and output. A write buffer will only execute one write per connection call

class Connection {
public:
	Request request;
	bool syscalled;

~Connection() {
	if (request.fd != -1)
		close(request.fd);
}

void init(i32 fd) {
	request.fd = fd;
}

void clear() {
}

isize dispatch(usize bytes, u32 events) {
	isize rvalue = 0;

	while (syscalled == false)
	{
		switch (request.state)
		{
			case HTTP::Attributes::READING:
				rvalue = request.parse_header(bytes, events);
				break;
			case HTTP::Attributes::PROCESSING:
				rvalue = request.parse_body(bytes, events);
				break;
			case HTTP::Attributes::WRITING:
				rvalue = request.upload(bytes, events);
				break;
			default: break;
		}
	}
	syscalled = false;	// TODO: Change the bitset functions
	return rvalue;
}
};
}

#pragma once

#include <unistd.h>

#include "core.hpp"
#include "Request.hpp"

namespace HTTP {

class Connection {
public:
	Request request;

	~Connection() {
		if (request.fd != -1)
			close(request.fd);
	}

	void	init(i32 fd) {
		request.fd = fd;
	}

		// its called by ServerManager.run().
	// return values:
	// 		REQ_CONTINUE: response is not ready.
	// 		REQ_CLOSE:	  close the connection (will remove fd from epoll)
	// 		REQ_WRITE:	  says you're ready to write the response and ServerManager will modify its epoll so that you write
	//
	// TODO: check when to return REQ_CLOSE. Maybe should check for events & (EPOLLHUP | EPOLLERR) to close the connection properly.
	// Maybe should close according to something in the request? Maybe need to close if EPOLLIN && (rvalue = read()) == 0
	i8	handleEvent(usize bytes, u32 events)
	{
		i8 rvalue = 0;

		while (request.syscalled == false)
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
					rvalue = request.exec(bytes, events);
					break;
				default: break;
			}
		}
		request.syscalled = false;	// TODO: Change the bitset functions
		return rvalue;
	}
};
}

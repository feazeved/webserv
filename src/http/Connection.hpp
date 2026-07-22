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

};
}

#pragma once

#include <unistd.h>
#include "HTTP.hpp"
#include "State.hpp"
#include "core.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <netdb.h>

namespace HTTP {

#define VIRTUALSERVER_INL(ret_type) ret_type inline HTTP::VirtualServer::

class VirtualServer {
public:
	StringView			clientErrors[32];
	StringView			serverErrors[12];
	StringView			host;
	Array32<Location>	locations;
	usize				port;
	usize				maxBodySize;
	Game::State			*gameState;
	i32 				listenFd;

	VirtualServer();

	~VirtualServer() {
		clear();
	}

	int clear() {
		if (listenFd != -1) {
			close(listenFd);
			listenFd = -1;
		}
		gameState = NULL;
		return 1;
	}

	static bool s_set_socket_nonblocking(i32 fd) {
		i32 flags = fcntl(fd, F_GETFL, 0);
		return flags == -1 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1;
	}

	void init() {
		if (listenFd != -1)
			clear();

		if (port < 1 || port > 65535)
			PERR_EXIT(clear(), "Error: Invalid virtual server port");

		listenFd = socket(AF_INET, SOCK_STREAM, 0);
		if (listenFd == -1)
			PERR_EXIT(clear(), "Error: Failed to create listening socket");

		i32 reuseAddress = 1;
		if (setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR,
			&reuseAddress, sizeof(reuseAddress)) == -1)
			PERR_EXIT(clear(), "Error: Failed to configure listening socket");

		sockaddr_in address;
		if (s_resolve_host_and_port(host, port, address))
			PERR_EXIT(clear(), "Error: Failed to resolve virtual server host");
		if (bind(listenFd, (sockaddr*) &address, sizeof(address)) == -1)
			PERR_EXIT(clear(), "Error: Failed to bind listening socket");
		if (listen(listenFd, SOMAXCONN) == -1)
			PERR_EXIT(clear(), "Error: Failed to listen on socket");
		if (s_set_socket_nonblocking(listenFd))
			PERR_EXIT(clear(), "Error: Failed to make listening socket non-blocking");
	}

	bool cache_error_pages();

	static bool s_resolve_host_and_port(const HTTP::StringView& host, usize port, sockaddr_in& address) {
		addrinfo hints;
		MEMSET_INLINE(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = 0;

		addrinfo* result = NULL;
		i32 status = getaddrinfo(host.get(), NULL, &hints, &result);
		if (status != 0 || result == NULL)
			return true;

		bool invalid = result->ai_addrlen < sizeof(sockaddr_in);
		if (!invalid) {
			MEMSET_INLINE(&address, 0, sizeof(address));
			MEMCPY_INLINE(&address, result->ai_addr, sizeof(address));
			address.sin_port = htons((u16) port);
		}
		freeaddrinfo(result);
		return invalid;
	}
};

}
#pragma once
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <netdb.h>

#include "core.hpp"
#include "webserv.hpp"
#include "Span.hpp"
#include "Status.hpp"
#include "Array.hpp"
#include "Environment.hpp"

#define VIRTUALSERVER_INL(ret_type) ret_type inline VirtualServer::

class VirtualServer {
public:
	Span			serverRoot;
	Span			errorPages[Status::errorPageCount];
	Span			host;
	Array<Location>	locations;
	usize			port;
	usize			maxBodySize;
	void*			gameState;
	int 			listenFd;

	VirtualServer()
		: serverRoot(), host(), locations(), port(SIZE_MAX),
		  maxBodySize(SIZE_MAX), gameState(NULL), listenFd(-1) {
		MEMSET_INLINE(errorPages, 0, sizeof(errorPages));
	}

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

	void init() {
		if (listenFd != -1)
			clear();

		if (port < 1 || port > 65535)
			PERR_EXIT(clear(), "Error: Invalid virtual server port");

		listenFd = socket(AF_INET, SOCK_STREAM, 0);
		if (listenFd == -1)
			PERR_EXIT(clear(), "Error: Failed to create listening socket");
		if (s_set_close_on_exec(listenFd))	// TODO: Review
			PERR_EXIT(clear(), "Error: Failed to configure listening socket");

		int reuseAddress = 1;
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
		if (s_set_nonblocking(listenFd))
			PERR_EXIT(clear(), "Error: Failed to make listening socket non-blocking");
	}

	static inline
	bool s_set_nonblocking(int fd)
	{
		int flags = fcntl(fd, F_GETFL, 0);
		if (flags == -1)
			return true;

		if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
			return true;

		return false;
	}

	// TODO: Review
	static inline
	bool s_set_close_on_exec(int fd)
	{
		int flags = fcntl(fd, F_GETFD, 0);
		if (flags == -1)
			return true;
		return fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1;
	}

	static bool s_resolve_host_and_port(const Span& host, usize port, sockaddr_in& address) {
		addrinfo hints;
		MEMSET_INLINE(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = 0;

		addrinfo* result = NULL;
		int status = getaddrinfo(host.ptr, NULL, &hints, &result);
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

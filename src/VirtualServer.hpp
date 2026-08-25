#pragma once
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <netdb.h>

#include "core.hpp"
#include "webserv.hpp"
#include "StringView.hpp"
#include "Status.hpp"
#include "Array.hpp"
#include "VirtualServer_helpers.ipp"

#define VIRTUALSERVER_INL(ret_type) ret_type inline VirtualServer::

class VirtualServer {
public:
	StringView32		serverRoot;
	StringView32		errorPages[Status::errorPageCount];
	StringView32		host;
	Array32<Location>	locations;
	usize				port;
	usize				maxBodySize;
	void*				gameState;
	int 				listenFd;

	VirtualServer()
		: serverRoot(), host(), locations(), port(SIZE_MAX),
		  maxBodySize(SIZE_MAX), gameState(NULL), listenFd(-1) {}

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
		if (s_set_socket_nonblocking(listenFd))
			PERR_EXIT(clear(), "Error: Failed to make listening socket non-blocking");
	}
};

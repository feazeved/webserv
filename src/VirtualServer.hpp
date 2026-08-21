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

class VirtualServer {
public:
	HTTP::ServerConfig cfg;
	i32 listenFd;
	Game::State gameState;

	VirtualServer() : cfg(), listenFd(-1), gameState() {}

	~VirtualServer() {
		cleanup();
	}

	int cleanup() {
		if (listenFd != -1) {
			close(listenFd);
			listenFd = -1;
		}
		cfg.gameState = NULL;
		return 1;
	}

	static bool s_set_socket_nonblocking(i32 fd) {
		i32 flags = fcntl(fd, F_GETFL, 0);
		return flags == -1 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1;
	}

	void init() {
		if (listenFd != -1)
			cleanup();
		cfg.gameState = &gameState;

		if (cfg.port < 1 || cfg.port > 65535)
			PERR_EXIT(cleanup(), "Error: Invalid virtual server port");

		listenFd = socket(AF_INET, SOCK_STREAM, 0);
		if (listenFd == -1)
			PERR_EXIT(cleanup(), "Error: Failed to create listening socket");

		i32 reuseAddress = 1;
		if (setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR,
			&reuseAddress, sizeof(reuseAddress)) == -1)
			PERR_EXIT(cleanup(), "Error: Failed to configure listening socket");

		sockaddr_in address;
		if (s_resolve_host_and_port(cfg.host, cfg.port, address))
			PERR_EXIT(cleanup(), "Error: Failed to resolve virtual server host");
		if (bind(listenFd, (sockaddr*) &address, sizeof(address)) == -1)
			PERR_EXIT(cleanup(), "Error: Failed to bind listening socket");
		if (listen(listenFd, SOMAXCONN) == -1)
			PERR_EXIT(cleanup(), "Error: Failed to listen on socket");
		if (s_set_socket_nonblocking(listenFd))
			PERR_EXIT(cleanup(), "Error: Failed to make listening socket non-blocking");
	}

	static bool s_resolve_host_and_port(const HTTP::StringView& host, usize port, sockaddr_in& address) {
		char hostBuffer[257];
		const char* hostPtr = "0.0.0.0";

		if (host.length != 0) {
			if (host.length > sizeof(hostBuffer) - 1)
				return true;
			MEMCPY(hostBuffer, host.get(), host.length);
			hostBuffer[host.length] = 0;
			hostPtr = hostBuffer;
		}

		addrinfo hints;
		MEMSET_INLINE(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = 0;

		addrinfo* result = NULL;
		i32 status = getaddrinfo(hostPtr, NULL, &hints, &result);
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
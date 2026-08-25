#pragma once
#include "core.hpp"
#include "VirtualServer.hpp"

static bool s_set_socket_nonblocking(int fd) {
	int flags = fcntl(fd, F_GETFL, 0);
	return flags == -1 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1;
}

static bool s_resolve_host_and_port(const StringView32& host, usize port, sockaddr_in& address) {
	addrinfo hints;
	MEMSET_INLINE(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0;

	addrinfo* result = NULL;
	int status = getaddrinfo(host.kptr(), NULL, &hints, &result);
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

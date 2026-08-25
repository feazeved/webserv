#pragma once
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <cerrno>
#include <unistd.h>

#include "Server.hpp"

SERVER_INL
(void) mark_connection_writable(usize connectionIndex) {
	modify_epoll_event(connectionIndex, EPOLLOUT);
}

SERVER_INL
(void) add_to_epoll(int fd, u32 events, u64 key) {
	struct epoll_event event;
	MEMSET_INLINE(&event, 0, sizeof(event));
	event.events = events;
	event.data.u64 = key;
	if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &event) == -1)
		PERR_EXIT(clear(), "Error: Failed to add epoll event");
}

SERVER_INL
(void) remove_from_epoll(int fd) {
	(void) epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, NULL);
}

SERVER_INL
(void) modify_epoll_event(usize connectionIndex, u32 events) {
	struct epoll_event event;
	MEMSET_INLINE(&event, 0, sizeof(event));
	event.events = events;
	event.data.u64 = s_epoll_connection_key(connectionIndex);
	if (epoll_ctl(epollFd, EPOLL_CTL_MOD,
		connections[connectionIndex].clientFd, &event) == -1)
		PERR_EXIT(clear(), "Error: Failed to modify epoll event");
}

SERVER_INL
(void) add_connection(VirtualServer* server) {
	sockaddr_in clientAddress;
	socklen_t clientLength = sizeof(clientAddress);
	int clientFd = accept(server->listenFd, (sockaddr*) &clientAddress,
		&clientLength);
	if (clientFd == -1) {
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
			return;
		PERR_RETURN((void)0, "Error: Failed to accept connection");
	}
	if (s_set_socket_nonblocking(clientFd)) {
		close(clientFd);
		PERR_RETURN((void)0, "Error: Failed to make client socket non-blocking");
	}

	usize index = connections.acquire_slot(clientFd, server);	// TODO: Check what we do here
	if (index == SIZE_MAX) {
		close(clientFd);
		PERR_RETURN((void)0, "Error: Connection capacity reached");
	}
	add_to_epoll(clientFd, EPOLLIN, s_epoll_connection_key(index));
}

SERVER_INL
(void) close_connection(usize connectionIndex) {
	remove_from_epoll(connections[connectionIndex].clientFd);
	connections.free_slot(connectionIndex);
}

#pragma once

#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <cerrno>
#include <unistd.h>

#include "Server.hpp"

SERVER_INL
(void) mark_connection_writable(i32 fd, void* conn) {
	modify_epoll_event(fd, EPOLLOUT, conn);
}

SERVER_INL
(void) add_to_epoll(i32 fd, u32 events, void* ptr) {
	u64 key = 0;
	bool found = false;
	for (usize index = 0; index < serverCount; index++) {
		if (ptr == &servers[index]) {
			key = s_epoll_server_key(index);
			found = true;
			break;
		}
	}
	if (!found) {
		usize index = connections.index_of((HTTP::Connection*) ptr);
		if (index == SIZE_MAX)
			PERR_EXIT(cleanup(), "Error: Invalid epoll registration");
		key = s_epoll_connection_key(index);
	}

	struct epoll_event event;
	MEMSET_INLINE(&event, 0, sizeof(event));
	event.events = events;
	event.data.u64 = key;
	if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &event) == -1)
		PERR_EXIT(cleanup(), "Error: Failed to add epoll event");
}

SERVER_INL
(void) remove_from_epoll(i32 fd) {
	(void) epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, NULL);
}

SERVER_INL
(void) modify_epoll_event(i32 fd, u32 events, void* ptr) {
	usize index = connections.index_of((HTTP::Connection*) ptr);
	if (index == SIZE_MAX)
		return;

	struct epoll_event event;
	MEMSET_INLINE(&event, 0, sizeof(event));
	event.events = events;
	event.data.u64 = s_epoll_connection_key(index);
	if (epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &event) == -1)
		PERR_EXIT(cleanup(), "Error: Failed to modify epoll event");
}

SERVER_INL
(void) add_connection(VirtualServer* server) {
	sockaddr_in clientAddress;
	socklen_t clientLength = sizeof(clientAddress);
	i32 clientFd = accept(server->listenFd, (sockaddr*) &clientAddress,
		&clientLength);
	if (clientFd == -1) {
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
			return;
		PERR_RETURN((void)0, "Error: Failed to accept connection");
	}
	if (VirtualServer::s_set_socket_nonblocking(clientFd)) {
		close(clientFd);
		PERR_RETURN((void)0, "Error: Failed to make client socket non-blocking");
	}

	usize index = connections.acquire_slot();
	if (index == SIZE_MAX) {
		close(clientFd);
		if (connections.capacity() < connections.maxElements)
			PERR_EXIT(cleanup(), "Error: Failed to grow connection storage");
		PERR_RETURN((void)0, "Error: Connection capacity reached");
	}
	connections[index].init(clientFd, &server->cfg);
	add_to_epoll(clientFd, EPOLLIN, connections.get(index));
}

SERVER_INL
(void) close_connection(HTTP::Connection* connection) {
	usize index = connections.index_of(connection);
	if (index == SIZE_MAX)
		return;
	remove_from_epoll(connection->clientFd);
	connections.clear(index);
}


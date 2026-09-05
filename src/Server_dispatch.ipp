#pragma once
#include "Server.hpp"

SERVER_INL
(void) add_connection(u32 serverIndex) {
	VirtualServer *server = &servers[serverIndex];
	sockaddr_in clientAddress;
	socklen_t clientLength = sizeof(clientAddress);
	int clientFd = accept(server->listenFd, (sockaddr*) &clientAddress, &clientLength);
	if (clientFd == -1) {
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
			return;
		PERR_RETURN((void)0, "Error: Failed to accept connection");
	}
	if (fn::set_stream_mode(clientFd)) {
		close(clientFd);
		PERR_RETURN((void)0, "Error: Failed to make client socket non-blocking");
	}

	const usize connectionIndex = connections.acquire_slot(clientFd, server);
	if (connectionIndex == SIZE_MAX) {
		close(clientFd);
		PERR_RETURN((void)0, "Error: Connection capacity reached");
	}
	if (epoll.add(clientFd, EPOLLIN, (u32)connectionIndex, serverIndex)) {
		connections[connectionIndex].end_connection();
		connections.free_slot(connectionIndex);
		PERR_RETURN((void)0, "Error: Failed to add client event");
	}
}

SERVER_INL
(void) remove_connection(u32 connectionIndex) {
	epoll.remove(connections[connectionIndex].clientFd);
	connections.free_slot(connectionIndex);
	connections.mark_for_deletion(connectionIndex);
}

SERVER_INL
(void) server_event(u64 key) {
	const u32 serverIndex = (u32)(key >> 32);
	if (serverIndex >= parser.serverCount)
		PERR_EXIT(clear(), "Error: Invalid listening socket event");
	if (epoll.is_error())
		PERR_EXIT(clear(), "Error: Listening socket failed");
	if (epoll.is_readable())
		add_connection(serverIndex);
}

SERVER_INL
(void) connection_event(u64 key) {
	const u32 connectionIndex = (u32)key;
	if (epoll.is_error()) {
		remove_connection(connectionIndex);
		return;
	}
	if (connections[connectionIndex].dispatch(epoll) == -1)
		remove_connection(connectionIndex);
}

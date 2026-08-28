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
	if (VirtualServer::s_set_nonblocking(clientFd)
		|| VirtualServer::s_set_close_on_exec(clientFd)) {
		close(clientFd);
		PERR_RETURN((void)0, "Error: Failed to make client socket non-blocking");
	}

	const usize connectionIndex = connections.acquire_slot(clientFd, server);
	if (connectionIndex == SIZE_MAX) {
		close(clientFd);
		PERR_RETURN((void)0, "Error: Connection capacity reached");
	}
	if (epoll.add(clientFd, EPOLLIN, (u32)connectionIndex, serverIndex)) {
		connections.free_slot(connectionIndex);
		PERR_RETURN((void)0, "Error: Failed to add client event");
	}
}

SERVER_INL
(void) close_connection(u32 connectionIndex) {
	epoll.remove(connections[connectionIndex].clientFd);
	connections.free_slot(connectionIndex);
}

SERVER_INL
(void) run() {
	for (u32 serverIndex = 0; serverIndex < parser.serverCount; serverIndex++) {
		if (epoll.add(servers[serverIndex].listenFd, EPOLLIN, UINT32_MAX, serverIndex))
			PERR_EXIT(clear(), "Error: Failed to add listening socket event");
	}

	struct epoll_event events[s_maxEvents];
	while (true) {
		const int eventCount = epoll.wait(events, s_maxEvents, 1000);
		if (eventCount == -1) {
			if (errno == EINTR)
				continue;
			PERR_EXIT(clear(), "Error: epoll_wait failed");
		}

		Clock::update_time();
		for (int eventIndex = 0; eventIndex < eventCount; eventIndex++) {
			epoll.event = events[eventIndex];
			if (epoll.is_server_event())
				server_event();
			else
				connection_event();
		}
		check_timeouts();
	}
}

SERVER_INL
(void) server_event() {
	const u32 serverIndex = epoll.server_index();
	if (serverIndex >= parser.serverCount)
		PERR_EXIT(clear(), "Error: Invalid listening socket event");
	if (epoll.event.events & (EPOLLERR | EPOLLHUP))
		PERR_EXIT(clear(), "Error: Listening socket failed");
	if (epoll.event.events & EPOLLIN)
		add_connection(serverIndex);
}

SERVER_INL
(void) connection_event() {
	const u32 connectionIndex = epoll.connection_index();
	if (epoll.event.events & (EPOLLERR | EPOLLHUP)) {
		close_connection(connectionIndex);
		return;
	}
	if (connections[connectionIndex].dispatch(epoll) == -1)
		close_connection(connectionIndex);
}

// SERVER_INL
// (void) check_timeouts() {
// 	while (true) {
// 		const usize connectionIndex = connections.find_timed_out(Clock::time_elapsed());
// 		if (connectionIndex == SIZE_MAX)
// 			return;
// 		close_connection((u32)connectionIndex);
// 	}
// }

#pragma once
#include "Server.hpp"

static inline
u64 s_epoll_server_key(usize index) {
	return ((u64) index << 1) | 1;
}

SERVER_INL
(void) run() {
	for (usize index = 0; index < parser.serverCount; index++)
		add_to_epoll(servers[index].listenFd, EPOLLIN, s_epoll_server_key(index));

	struct epoll_event events[s_maxEvents];
	while (true) {
		int eventCount = epoll_wait(epollFd, events, s_maxEvents, -1);
		if (eventCount == -1) {
			if (errno == EINTR)
				continue;
			PERR_EXIT(clear(), "Error: epoll_wait failed");
		}
		for (int index = 0; index < eventCount; index++)
			dispatch_epoll_event(events[index]);
	}
}

static inline
usize s_epoll_key_index(u64 key) {
	return (usize)(key >> 1);
}

static inline
bool s_epoll_key_is_server(u64 key) {
	return (key & 1) != 0;
}

SERVER_INL
(void) dispatch_epoll_event(const struct epoll_event& event) {
	u64 key = event.data.u64;
	usize index = s_epoll_key_index(key);
	if (!s_epoll_key_is_server(key)) {
		dispatch_connection_event(index, event.events);
		return;
	}
	if (index >= parser.serverCount)
		PERR_EXIT(clear(), "Error: Invalid listening socket event");
	if (event.events & (EPOLLERR | EPOLLHUP))
		PERR_EXIT(clear(), "Error: Listening socket failed");
	if (event.events & EPOLLIN)
		add_connection(&servers[index]);
}

SERVER_INL
(void) dispatch_connection_event(usize index, u32 events) {
	isize result = connections[index].dispatch(events);
	if (result == 0)
		close_connection(index);
	else if (result == 2)
		modify_epoll_event(index, EPOLLOUT);
}

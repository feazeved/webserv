#pragma once
#include "Server.hpp"

SERVER_INL
(void) run() {
	for (usize index = 0; index < parser.serverCount; index++) {
		if (add_to_epoll(servers[index].listenFd, EPOLLIN, s_epoll_server_key(index)))
			PERR_EXIT(clear(), "Error: Failed to add listening socket event");
	}
	struct epoll_event events[s_maxEvents];
	while (true) {
		int eventCount = epoll_wait(epollFd, events, s_maxEvents, 1000);	// TODO: Review 1000
		if (eventCount == -1) {
			if (errno == EINTR)
				continue;
			PERR_EXIT(clear(), "Error: epoll_wait failed");
		}
		Clock::update_time();
		for (int index = 0; index < eventCount; index++) {
			dispatch_epoll_event(events[index]);
		}
		Clock::update_time();
		check_timeouts();
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
	if (event.events & (EPOLLERR | EPOLLHUP))
		PERR_EXIT(clear(), "Error: Listening socket failed");
	if (event.events & EPOLLIN)
		add_connection(&servers[index]);
}

SERVER_INL
(void) dispatch_connection_event(usize index, u32 events) {
	if (events & (EPOLLERR | EPOLLHUP)) {
		close_connection(index);
		return;
	}
	isize result = connections[index].dispatch(events);
	if (result <= 0)
		close_connection(index);
	else if (modify_epoll_event(index, (u32)result))
		close_connection(index);
}

// SERVER_INL
// (void) check_timeouts() {
// 	while (true) {
// 		const usize index = connections.find_timed_out(Clock::time_elapsed());
// 		if (index == SIZE_MAX)
// 			return;
// 		close_connection(index);
// 	}
// }

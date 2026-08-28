#pragma once
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>

#include "core.hpp"

struct Epoll {
	i32 fd;
	struct epoll_event event;

	bool init() {
		clear();
		fd = epoll_create(1);
		if (fd == -1)
			PERR_RETURN(true, "Error: Failed to create epoll instance");

		const int flags = fcntl(fd, F_GETFD, 0);
		if (flags == -1 || fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1) {
			clear();
			PERR_RETURN(true, "Error: Failed to configure epoll instance");
		}
		return false;
	}

	void clear() {
		if (fd != -1) {
			close(fd);
			fd = -1;
		}
		reset_event();
	}

	int wait(struct epoll_event *events, int maxEvents, int timeout) {
		return epoll_wait(fd, events, maxEvents, timeout);
	}

	bool add(i32 targetFd, u32 events, u32 connectionIndex, u32 serverIndex) {
		assign(events, connectionIndex, serverIndex);
		return epoll_ctl(fd, EPOLL_CTL_ADD, targetFd, &event) == -1;
	}

	bool remove(i32 targetFd) {
		return epoll_ctl(fd, EPOLL_CTL_DEL, targetFd, NULL) == -1;
	}

	bool modify(i32 targetFd, u32 events) {
		event.events = events;
		return epoll_ctl(fd, EPOLL_CTL_MOD, targetFd, &event) == -1;
	}

	void assign(u32 events, u32 connectionIndex, u32 serverIndex) {
		reset_event();
		event.events = events;
		event.data.u64 = ((u64)serverIndex << 32) | (u64)connectionIndex;
	}

	u32 connection_index() const {
		return (u32)event.data.u64;
	}

	u32 server_index() const {
		return (u32)(event.data.u64 >> 32);
	}

	bool is_server_event() const {
		return connection_index() == UINT32_MAX;
	}

	void reset_event() {
		MEMSET_INLINE(&event, 0, sizeof(event));
	}

};

STATIC_ASSERT(sizeof(u64) == 2 * sizeof(u32));
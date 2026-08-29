#pragma once
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>

#include "core.hpp"
#include "VirtualServer.hpp"

struct Key {
	i32 clientFd;
	u32 index;

	bool is_server() {
		return !!(index & 0x80000000ul);
	}

	u32 server_index() {
		return index & 0x7FFFFFFFul;
	}

	u32 connection_index() {
		return index;
	}
};

struct Epoll {
	static const usize maxEvents = 64;

	i32 fd;
	usize index;
	VirtualServer* servers;
	struct epoll_event eventList[maxEvents];

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
		MEMSET_INLINE(eventList, 0, sizeof(eventList));
	}

	// Can return a personal struct here and access it like .fd, .clientIndex
	struct epoll_event* get_event(usize srcIndex) {
		index = srcIndex;
		return eventList + index;
	}

	usize wait(int timeout) {
		return (usize)epoll_wait(fd, eventList, maxEvents, timeout);
	}

	bool add(i32 targetFd, u32 events, u32 connectionIndex, u32 serverIndex) {
		eventList[index].events = events;
		eventList[index].data.u64 = ((u64)serverIndex << 32) | (u64)connectionIndex;
		return epoll_ctl(fd, EPOLL_CTL_ADD, targetFd, eventList + index) == -1;
	}

	bool remove(i32 targetFd) {
		return epoll_ctl(fd, EPOLL_CTL_DEL, targetFd, NULL) == -1;
	}

	bool set_write(i32 targetFd, bool bit) {
		if (bit)
			eventList[index].events |= EPOLLOUT;
		else
			eventList[index].events &= ~EPOLLOUT;
		return epoll_ctl(fd, EPOLL_CTL_MOD, targetFd, eventList + index) == -1;
	}

	bool set_read(i32 targetFd, bool bit) {
		if (bit)
			eventList[index].events |= EPOLLIN;
		else
			eventList[index].events &= ~EPOLLIN;
		return epoll_ctl(fd, EPOLL_CTL_MOD, targetFd, eventList + index) == -1;
	}

	bool is_writeable() {
		return !!(eventList[index].events & EPOLLOUT);
	}

	bool is_readable() {
		return !!(eventList[index].events & EPOLLIN);
	}

	bool is_error() {
		return !!(eventList[index].events & (EPOLLERR | EPOLLHUP));
	}

	// TBD
	int client_fd() {
		return (i32) eventList[index].data.u64;
	}

	VirtualServer* cfg() {
		return servers + index;
	}
};

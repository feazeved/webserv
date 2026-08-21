#pragma once

#include "core.hpp"

static inline
u64 s_epoll_server_key(usize index) {
	return ((u64) index << 1) | 1;
}

static inline
u64 s_epoll_connection_key(usize index) {
	return (u64) index << 1;
}

static inline
bool s_epoll_key_is_server(u64 key) {
	return (key & 1) != 0;
}

static inline
usize s_epoll_key_index(u64 key) {
	return (usize)(key >> 1);
}

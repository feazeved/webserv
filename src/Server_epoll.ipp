#pragma once

#include <stdexcept>
#include <sys/socket.h>
#include <vector>
#include <cstring>
#include <cerrno>
#include <sys/epoll.h>
#include <csignal>
#include <iostream>

#include "HTTP.hpp"
#include "State.hpp"
#include "Connection.hpp"
#include "BlockVector.hpp"
#include "core.hpp"
#include "Server.hpp"
#include "VirtualServer.hpp"

SERVER_INL
(void) mark_connection_writable(i32 fd, void* conn) {
	modify_epoll_event(fd, EPOLLOUT, conn);
}

SERVER_INL
(void) add_to_epoll(i32 fd, u32 events, void* ptr) {
	struct epoll_event ev;
	ev.events = events;
	ev.data.ptr = ptr;
	if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &ev) == -1)
		throw std::runtime_error(std::strerror(errno));
}

SERVER_INL
(void) remove_from_epoll(i32 fd) {
	epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, NULL);
}

SERVER_INL
(void) modify_epoll_event(i32 fd, u32 events, void* ptr) {
	struct epoll_event ev;
	ev.events = events;
	ev.data.ptr = ptr;
	if (epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &ev) == -1)
		std::cerr << "epoll_ctl MOD error: " << std::strerror(errno) << "\n";
}

SERVER_INL
(void) add_connection(VirtualServer* server) {
	sockaddr_in clientAddr;
	socklen_t clientLen = sizeof(clientAddr);

	i32 clientFd = accept(server->listenFd, (sockaddr*)&clientAddr, &clientLen);
	if (clientFd == -1) {
		if (errno != EAGAIN && errno != EWOULDBLOCK)
			std::cerr << "accept error: " << std::strerror(errno) << "\n";
		return;
	}

	if (fcntl(clientFd, F_SETFL, O_NONBLOCK) == -1) {
		std::cerr << "fcntl error: " << std::strerror(errno) << "\n";
		close(clientFd);
		return;
	}

	usize index = connections.find_free_slot();
	if (index == SIZE_MAX) {
		close(clientFd);
		throw std::bad_alloc();	// TODO: Throw
	}
	connections[index].init(clientFd, &server->config);
	add_to_epoll(clientFd, EPOLLIN | EPOLLOUT, &connections[index]);
}

SERVER_INL
(void) close_connection(HTTP::Connection* conn) {
	// if (conn->gameState)
	// 	conn->gameState->removeSSEClient(conn);
	remove_from_epoll(conn->clientFd);
}

SERVER_INL
(void) broadcast_all_server_events() {
	for (usize i = 0; i < servers.size(); i++) {
		servers[i].gameState.broadcastEvents(*this);
	}
}

SERVER_INL
(bool) is_listening_socket(void* ptr) {
	for (usize i = 0; i < servers.size(); i++) {
		if (ptr == &servers[i])
			return true;
	}
	return false;
}

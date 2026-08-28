#pragma once
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>

#include "core.hpp"
#include "webserv.hpp"
#include "Epoll.hpp"
#include "ConnectionPool.hpp"
#include "VirtualServer.hpp"
#include "Parser.hpp"
#include "Environment.hpp"

#define SERVER_INL(ret_type) ret_type inline Server::

class Server {
public:
	static const usize s_maxEvents = 16;
	VirtualServer servers[MAX_VIRTUAL_SERVERS];
	Parser parser;
	ConnectionPool connections;
	Epoll epoll;

	Server(const char *filePath, char **envp)
		: parser(filePath, servers) {

		VirtualServer::s_fakeEnv.init(envp);
		if (epoll.init())
			PERR_EXIT(clear(), "Error: Failed to create epoll instance");

		for (usize index = 0; index < parser.serverCount; index++)
			servers[index].init();
		Clock::init();
	}

	~Server() {
		clear();
	}

	int clear() {
		epoll.clear();
		connections.clear();
		for (usize index = 0; index < parser.serverCount; index++)
			servers[index].clear();
		parser.serverCount = 0;
		Arena::clear();
		return 1;
	}

	// Execution
	void run();
	void server_event();
	void connection_event();
	void check_timeouts();
	void add_connection(u32 serverIndex);
	void close_connection(u32 connectionIndex);
};

#include "Server_dispatch.ipp"

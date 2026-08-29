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

__attribute__((constructor))
void init(int argc, char** argv, char** envp) {
	(void) argc, (void)argv, (void) envp; 

	Clock::init();
	Environment::init(envp);
	// Memory related init stuff like MEMCPY_INLINE ARENA_STATIC_STRINGS
}

__attribute__((destructor))
void clear() {

}

#define SERVER_INL(ret_type) ret_type inline Server::
// if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
// 	PERR_RETURN(1, "Error: Failed to configure SIGPIPE handling");

// Have server be a template? up to 4096 connections
// Could then reasonably assign 64 connections per virtual server
class Server {
public:
	ConnectionPool connections;
	u8 storage[CONFIG_POOL_SIZE];
	Arena alpha, beta;

	VirtualServer servers[MAX_VIRTUAL_SERVERS];
	Parser parser;
	Epoll epoll;

	Server(const char *filePath) : alpha((u8*)&connections, sizeof(connections)), 
		beta(storage, sizeof(storage)), parser(filePath, servers, alpha, beta), epoll(servers) {
		if (epoll.fd == -1)
			PERR_EXIT(clear(), "Error: Failed to create epoll");

		for (usize index = 0; index < parser.serverCount; index++)
			servers[index].init();
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
		alpha.clear();
		beta.clear();
		return 1;
	}

	// Execution
	void run();
	void server_event(u64 key);
	void connection_event(u64 key);
	void check_timeouts();
	void add_connection(u32 serverIndex);
	void close_connection(u32 connectionIndex);
};

#include "Server_dispatch.ipp"

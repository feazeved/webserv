#pragma once
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>

#include "core.hpp"
#include "webserv.hpp"
#include "Epoll.hpp"
#include "ConnectionPool.hpp"
#include "VirtualServer.hpp"
#include "Parser.hpp"
#include "Clock.hpp"
#include "Environment.hpp"

__attribute__((constructor))
void init(int argc, char** argv, char** envp) {
	(void) argc, (void)argv, (void) envp; 

	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		PERR_EXIT(1, "Error: Failed to configure SIGPIPE handling");
	Clock::init();
	Environment::init(envp);
	// Memory related init stuff like MEMCPY_INLINE ARENA_STATIC_STRINGS
}

__attribute__((destructor))
void clear() {

}

#define SERVER_INL(ret_type) ret_type inline Server::
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

	// connections.for_each_active<Connection::check_timeout(30)>;	// how the fuck do you do this

	void check_timeouts() {
		pid_t pidList[ConnectionPool::elementCount];
		u32 indexList[ConnectionPool::elementCount];
		usize count = 0;
		time_t timeNow = Clock::update();
		usize elementIndex;
	
		for (usize i = 0; i < connections.elementCount; i++) {
			Bitmap bitmap = connections.elementBitmap[i];
			usize outerIndex = 64 * i;
			while ((elementIndex = bitmap.pop_first_set()) != WORD_BITS) {
				usize linearIndex = outerIndex + elementIndex;
				Connection& connection = connections.connections[linearIndex];
				if (timeNow - connection.startTime > 60) {
					pidList[count] = connection.processId;
					indexList[count] = linearIndex;
					kill(connection.processId, SIGKILL);
					remove_connection(linearIndex);
					count++;
				}
			}
		}

		for (usize i = 0; i < count; i++)
			waitpid(pidList[i], NULL, WNOHANG);	// Do i have to stop and wait for one of them?
	}

	// Execution
	void run();
	void server_event(u64 key);
	void connection_event(u64 key);
	void add_connection(u32 serverIndex);
	void remove_connection(u32 connectionIndex);
};

#include "Server_dispatch.ipp"

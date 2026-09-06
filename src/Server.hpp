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
		connections.reset();
		if (epoll.fd == -1)
			PERR_EXIT(clear(), "Error: Failed to create epoll");

		for (usize index = 0; index < parser.serverCount; index++)
			servers[index].init();
	}

	void run() {
		struct epoll_event* event;

		for (u32 serverIndex = 0; serverIndex < parser.serverCount; serverIndex++) {
			if (epoll.add(servers[serverIndex].listenFd, EPOLLIN, UINT32_MAX, serverIndex))
				PERR_EXIT(clear(), "Error: Failed to add listening socket event");
		}

		while (true) {
			const usize eventCount = epoll.wait(1000);
			if (eventCount == SIZE_MAX) {
				if (errno == EINTR)
					continue;
				PERR_EXIT(clear(), "Error: epoll_wait failed");
			}

			Clock::update();
			for (usize eventIndex = 0; eventIndex < eventCount; eventIndex++) {
				event = epoll.get_event(eventIndex);
				if ((u32)event->data.u64 == UINT32_MAX)
					server_event(event->data.u64);
				else
					connection_event(event->data.u64);
			}
			check_timeouts();
		}
	}

	void reap_children() {
		usize elementIndex;

		for (usize i = 0; i < connections.blockCount; i++) {
			Bitmap bitmap = connections.delBitmap[i];
			const usize outerIndex = 64 * i;

			while ((elementIndex = bitmap.pop_first_set()) != WORD_BITS) {
				const usize linearIndex = outerIndex + elementIndex;
				Connection& connection = connections.connections[linearIndex];

				if (connection.processId != -1) {
					const pid_t result = waitpid(connection.processId, NULL, WNOHANG);
					if (result == 0 || (result == -1 && errno == EINTR))
						continue;
				}

				connections.delBitmap[i].bitclr(elementIndex);
				connections.free_slot(linearIndex);
			}
		}
	}

	// connections.for_each_active<Connection::check_timeout(30)>;	// how the fuck do you do this
	void check_timeouts() {
		time_t timeNow = Clock::update();
		usize elementIndex;
	
		for (usize i = 0; i < connections.blockCount; i++) {
			Bitmap bitmap = connections.elementBitmap[i];
			bitmap.bitmap &= ~connections.delBitmap[i];
			usize outerIndex = 64 * i;
			while ((elementIndex = bitmap.pop_first_set()) != WORD_BITS) {
				usize linearIndex = outerIndex + elementIndex;
				if (timeNow - connections.connections[linearIndex].startTime > HTTP_TIMEOUT)
					connections.mark_for_deletion(linearIndex);
			}
		}
		reap_children();
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

	~Server() { clear(); }

	// Handling
	void server_event(u64 key);
	void connection_event(u64 key);
	void add_connection(u32 serverIndex);
	void remove_connection(u32 connectionIndex);
};

#include "Server_dispatch.ipp"

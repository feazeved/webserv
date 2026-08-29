#define MAIN_FILE

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "core.hpp"
#include "Server.hpp"

int g_epollFd = -1;

int	main(int argc, char** argv, char** envp)
{
	if (argc != 2)
		PERR_RETURN(1, "Error: Usage -> ./webserv <config_file>");

	static Server server(argv[1], envp);
	server.run();
	return 0;
}

__attribute__((constructor))
void init() {
	// Memory related init stuff like MEMCPY_INLINE ARENA_STATIC_STRINGS
}

__attribute__((destructor))
void clear() {

}

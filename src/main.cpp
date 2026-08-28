#define MAIN_FILE

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "core.hpp"
#include "Server.hpp"

int g_epollFd = -1;

int	main(int argc, char** argv, char **envp)
{
	if (argc != 2)
		PERR_RETURN(1, "Error: Usage -> ./webserv <config_file>");

	// if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
	// 	PERR_RETURN(1, "Error: Failed to configure SIGPIPE handling");
	Server server(argv[1], envp);
	server.run();
	return 0;
}

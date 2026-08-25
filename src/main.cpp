#define MAIN_FILE

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "core.hpp"

#include "Server.hpp"

int g_epollFd = -1;

int	main(int argc, char** argv)
{
	if (argc != 2)
		PERR_RETURN(1, "Error: Usage -> ./webserv <config_file>");

	Server server(argv[1]);

}

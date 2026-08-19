#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <new>
#include "core.hpp"
#include "HTTP.hpp"

int g_epollFd = -1;

char* init(const char *str);

int	main(int argc, char** argv)
{
	if (argc != 2)
		PERR_RETURN(1, "Error: Usage -> ./webserv <config_file>");

	init(argv[1]);

}

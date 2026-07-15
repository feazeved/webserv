#include "Server.hpp"

Server::Server(const Http::ServerConfig& c) :
	config(c), listenFd(-1)
{
	boot();
}

// Should close socket
Server::~Server()
{
}

// Should open socket
void	Server::boot() const
{
	(void)listenFd;
}

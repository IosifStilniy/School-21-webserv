#include "Server.hpp"

Server::Server(void)
	: host("127.0.0.1"), port(htons(1024))
{
}

Server::~Server()
{
}

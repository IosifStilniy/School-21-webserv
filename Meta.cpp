#include "Meta.hpp"
#include <iostream>

Meta::Meta(void)
{
};

Meta::Meta(std::string const & address, int port): address(address), port(port), proto_num(getprotobyname("tcp")->p_proto), backlog(1)
{
	int	reuse_option = 1;
	int	buf;
	socklen_t len;

	this->listen_sock = socket(PF_INET, SOCK_STREAM, this->proto_num);
	std::memset(&this->server_addr, 0, sizeof(this->server_addr));
	server_addr.sin_addr.s_addr = inet_addr(address.c_str());
	server_addr.sin_family = PF_INET;
	server_addr.sin_port = htons(port);
	setsockopt(this->listen_sock, SOL_SOCKET, SO_REUSEADDR, &reuse_option, sizeof(reuse_option));
	if (bind(listen_sock, reinterpret_cast<sockaddr *>(&server_addr), sizeof(server_addr)))
		throw std::runtime_error(strerror(errno));
	fcntl(this->listen_sock, F_SETFL, O_NONBLOCK);
}

Meta::~Meta()
{
}

#ifndef META_HPP
# define META_HPP

# include <stdexcept>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <netdb.h>
# include <string>
# include <cstring>
# include <fstream>
# include <map>

# include "webserv.hpp"
# include "Location.hpp"

class Meta
{
	private:
	public:
		std::string						address;
		int								port;
		int								proto_num;
		int								backlog;
		sockaddr_in						server_addr;
		int								listen_sock;

		Meta(void);

	public:
		Meta(std::string const & address, int port);
		~Meta();
};

#endif
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
# include <vector>

# include "utils.hpp"

# include "Server.hpp"

class Meta
{
	public:
		std::string			address;
		int					port;
		int					proto_num;
		int					backlog;
		sockaddr_in			server_addr;
		int					listen_sock;
		std::vector<Server>	servers;

		Meta(void);

	private:
		static const std::vector<std::string>	_keywords;

	public:
		Meta(std::string const & conf_file);
		Meta(std::string const & address, int port);
		~Meta();
		
	
	private:
		void	_parseServerBlock(std::ifstream & conf, Server & server, size_t & line_counter);
		void	_parseLocationBlock(std::ifstream & conf, Location & location, size_t & line_counter);
};

#endif
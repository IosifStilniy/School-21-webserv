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
# include <fcntl.h>

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

		struct ParsedEntity
		{
			std::map<std::string, std::vector<std::string> >	params;
			std::map<std::string, ParsedEntity>					locations;
		};

	public:
		Meta(std::string const & conf_file);
		Meta(std::string const & address, int port);
		~Meta();
		
	
	private:
		void	_parseBlock(std::ifstream & conf, ParsedEntity & entity, size_t & line_counter);
		void	_readConfLine(std::ifstream & file, std::queue<ParsedEntity> & parsed_servers, size_t & line_counter);
		void	_prepareServer(ParsedEntity & p_server, Server & server);
};

#endif
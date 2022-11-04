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

# ifndef DEF_ADDR
#  define DEF_ADDR "127.0.0.1"
# endif

# ifndef DEF_PORT
#  define DEF_PORT "1024"
# endif

# ifndef DEF_BACKLOG
#  define DEF_BACKLOG 32
# endif

class Meta
{
	public:
		typedef	std::map<std::string, Server>	servers_type;

		int				proto_num;
		servers_type	servers;

	private:
		static const std::vector<std::string>	_keywords;

		struct ParsedEntity
		{
			std::map<std::string, std::string>	params;
			std::map<std::string, ParsedEntity>	locations;
		};

	public:
		Meta(void);
		Meta(std::ifstream & conf);
		~Meta();
		
		void	configurateServers(std::ifstream & conf);

	private:
		void				_parseBlock(std::ifstream & conf, ParsedEntity & entity, size_t & line_counter);
		void				_readConfLine(std::ifstream & file, std::queue<ParsedEntity> & parsed_servers, size_t & line_counter);
		void				_prepareServer(ParsedEntity & p_server, Server::Settings & server);
		void				_prepareLocation(ParsedEntity & p_server, ParsedEntity & p_location, Server::Location & location);
		std::string const &	_chooseSource(ParsedEntity & p_server, ParsedEntity & p_location, std::string const & param);
		void				_bindErrorPages(std::string const & params, std::map<int, std::string> & error_pages);

		static std::vector<std::string>	_getAllMethods(void);
};

#endif
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

# ifndef THROW_UNEXP_LINE
#  define THROW_UNEXP_LINE(line_num, line, text) (throw std::logic_error(std::string("unexpected line " + ft::num_to_string(line_num) + ": " + line + text).c_str()))
# endif

class Meta
{
	public:
		typedef	std::map<std::string, Server>	servers_type;

		int				proto_num;
		servers_type	servers;

	private:
		static std::vector<std::string>	_keywords;

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
		void	_parseBlock(std::ifstream & conf, ParsedEntity & entity, size_t & line_counter);
		void	_readConfLine(std::ifstream & file, std::queue<ParsedEntity> & parsed_servers, size_t & line_counter);
		void	_prepareServer(ParsedEntity & p_server, Server::Settings & server);
		void	_prepareLocation(ParsedEntity & p_location, Location & location);
		void	_checkLocations(Location const & def_loc, Server::locations_type const & locations);
		void	_bindErrorPages(std::string const & params, std::map<int, std::string> & error_pages);

		static std::vector<std::string>	_getAllMethods(void);

		static void	_initPath(std::string const & prog_name);
};

#endif
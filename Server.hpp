#ifndef SERVER_HPP
# define SERVER_HPP

# include <string>
# include <vector>
# include <map>

# include "Location.hpp"

class Server
{
	private:
	public:
		std::string							server_name;
		std::map<std::string, Location>		locations;
		std::string							host;
		size_t								port;

	public:
		Server(void);
		~Server();
};

#endif

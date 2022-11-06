#ifndef SERVER_HPP
# define SERVER_HPP

# include <string>
# include <vector>
# include <map>
# include <utility>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <fcntl.h>

# include "typedefs.hpp"
# include "Polls.hpp"
# include "RequestCollector.hpp"
# include "ResponseHandler.hpp"
# include "Maintainer.hpp"

# include "utils.hpp"

class Server
{
	public:
		typedef ByteTypes::byte_type		byte_type;
		typedef	ByteTypes::bytes_type		bytes_type;
		typedef ByteTypes::chunks_type		chunks_type;
		typedef ByteTypes::bytes_iterator	bytes_iterator;
		
	public:

		typedef std::map<std::string, Location>		locations_type;
		typedef ServerSettings						Settings;
		typedef ServerSettings::hosts_ports_type	hosts_ports_type;

	private:
	
	public:
		std::vector<Settings>	settings;
		ft::key_value_type		host_port;
		int						backlog;

		Polls					polls;

		RequestCollector		req_coll;
		Maintainer				maintainer;
		ResponseHandler			resp_handler;

		void	initialize(std::string const & host_port, int proto_num);
		void	startListen(void);
		void	proceed(int timeout = -1);

	public:
		Server(void);
		~Server();
};

#endif

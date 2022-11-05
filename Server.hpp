#ifndef SERVER_HPP
# define SERVER_HPP

# include <string>
# include <vector>
# include <map>
# include <utility>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <fcntl.h>

# include "Polls.hpp"
# include "RequestCollector.hpp"
# include "ResponseHandler.hpp"
# include "Maintainer.hpp"

class Server
{
	private:
	public:

		struct Location
		{
			std::string						root;
			std::vector<std::string>		indexes;
			std::vector<std::string>		methods;
			std::string						redir;
			std::string						e_is_dir;
			std::map<int, std::string>		error_pages;
			std::map<std::string, Location>	locations;
			size_t							buf_size;
		};

		typedef std::pair<std::string, std::string>		host_port_type;
		typedef	std::pair<host_port_type, int>			listen_params_type;
		typedef std::map<std::string, Location>			locations_type;
		typedef	std::vector<listen_params_type>			hosts_ports_type;

		struct Settings
		{
			std::vector<std::string>	server_names;
			locations_type				locations;
			Location					def_settings;
			hosts_ports_type			host_port;
		};
	
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

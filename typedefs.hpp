#ifndef TYPEDEFS_HPP
# define TYPEDEFS_HPP

# include <vector>
# include <queue>
# include <utility>
# include <map>
# include <string>
# include <fstream>

# include "utils.hpp"

namespace ft
{
	std::string	path;
}

struct ByteTypes
{
	typedef char						byte_type;
	typedef	std::vector<byte_type>		bytes_type;
	typedef std::queue<bytes_type>		chunks_type;
	typedef bytes_type::const_iterator	bytes_iterator;
};

struct Location
{
	typedef std::map<std::string, Location>	locations_type;

	std::string					root;
	std::vector<std::string>	indexes;
	std::vector<std::string>	methods;
	std::string					redir;
	std::string					e_is_dir;
	std::map<int, std::string>	error_pages;
	locations_type				locations;
	size_t						buf_size;
};

struct ServerSettings
{
	typedef std::pair<std::string, std::string>		host_port_type;
	typedef	std::pair<host_port_type, int>			listen_params_type;
	typedef	std::vector<listen_params_type>			hosts_ports_type;

	static std::vector<std::string>	supported_protos;

	std::vector<std::string>	server_names;
	Location					def_settings;
	hosts_ports_type			host_port;
};

#endif

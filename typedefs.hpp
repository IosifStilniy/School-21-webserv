#ifndef TYPEDEFS_HPP
# define TYPEDEFS_HPP

# include <vector>
# include <queue>
# include <utility>
# include <map>
# include <set>
# include <string>
# include <fstream>

# include "utils.hpp"

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
	std::set<std::string>		indexes;
	std::set<std::string>		methods;
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
	typedef	std::set<listen_params_type>			hosts_ports_type;

	static std::vector<std::string>	supported_protos;

	std::set<std::string>	server_names;
	Location				def_settings;
	hosts_ports_type		host_port;
};

#endif

#ifndef TYPEDEFS_HPP
# define TYPEDEFS_HPP

# include <vector>
# include <queue>
# include <utility>
# include <map>
# include <list>
# include <set>
# include <string>
# include <fstream>

# include "utils.hpp"

struct ByteTypes
{
	typedef char						byte_type;
	typedef	std::vector<byte_type>		bytes_type;
	typedef std::list<bytes_type>		chunks_type;
};

struct Location
{
	typedef std::map<std::string, Location>	locations_type;

	Location(void)
	{};

	~Location()
	{};

	std::string 				cgi;
	std::string					root;
	std::set<std::string>		indexes;
	std::set<std::string>		methods;
	std::pair<int, std::string>	redir;
	std::string					e_is_dir;
	std::map<int, std::string>	error_pages;
	locations_type				locations;
	size_t						size_limit;
};

struct ServerSettings
{
	typedef std::pair<std::string, std::string>		host_port_type;
	typedef	std::pair<host_port_type, int>			listen_params_type;
	typedef	std::set<listen_params_type>			hosts_ports_type;

	std::set<std::string>				server_names;
	std::map<std::string, std::string>	cgi;
	Location							def_settings;
	hosts_ports_type					host_port;
};

#endif

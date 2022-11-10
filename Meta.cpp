#include "Meta.hpp"
#include <iostream>

std::vector<std::string>	Meta::_keywords;

Meta::Meta(void)
	: proto_num(getprotobyname("tcp")->p_proto)
{
	if (Meta::_keywords.empty())
		Meta::_keywords = ft::containerazeConfFile<std::vector<std::string> >("info/keywords", &ft::returnLine);
};

Meta::Meta(std::ifstream & conf)
	: proto_num(getprotobyname("tcp")->p_proto)
{
	if (Meta::_keywords.empty())
		Meta::_keywords = ft::containerazeConfFile<std::vector<std::string> >("info/keywords", &ft::returnLine);

	this->configurateServers(conf);
};

Meta::~Meta()
{
}

void	Meta::configurateServers(std::ifstream & conf)
{
	size_t						line_counter = 0;
	std::queue<ParsedEntity>	parsed_servers;

	while (conf.good())
		this->_readConfLine(conf, parsed_servers, line_counter);

	if (!conf.eof())
		throw std::runtime_error(std::string("configuration file: " + std::string(strerror(errno))).c_str());
	
	if (conf.is_open())
		conf.close();
	
	while (!parsed_servers.empty())
	{
		Server::Settings	server;

		this->_prepareServer(parsed_servers.front(), server);
		parsed_servers.pop();

		for (Server::hosts_ports_type::const_iterator start = server.host_port.begin(); start != server.host_port.end(); start++)
			this->servers[start->first.first + ":" + start->first.second].settings.push_back(server);
	}

	for (servers_type::iterator start = this->servers.begin(); start != this->servers.end(); start++)
		start->second.initialize(start->first, this->proto_num);
}

std::vector<std::string>	Meta::_getAllMethods(void)
{
	std::vector<std::string>	methods;

	methods.push_back("get");
	methods.push_back("post");
	methods.push_back("delete");

	return (methods);
}

void	Meta::_bindErrorPages(std::string const & params, std::map<int, std::string> & error_pages)
{
	ft::splited_string	splited = ft::split(params);

	if (splited[0].empty())
		return ;

	if (splited.size() % 2)
		throw std::logic_error("'error_page' values must be pair '[error_num] [filename]'");

	std::ifstream	check;

	for (ft::splited_string::const_iterator key = splited.begin(), value = key + 1; key != splited.end(); key += 2, value += 2)
	{
		ft::openFile(check, *value);
		error_pages[strtol(key->c_str(), NULL, 10)] = *value;
	}

	if (check.is_open())
		check.close();
}

void	Meta::_prepareLocation(ParsedEntity & p_location, Location & location)
{
	location.root = ft::split(p_location.params["root"]).back();

	if (!location.root.empty() && location.root.back() != '/')
		location.root.push_back('/');

	ft::splited_string	splited = ft::split(p_location.params["indexes"]);

	if (splited[0] == "off")
		location.indexes.insert(std::string());
	else if (!splited[0].empty())
		location.indexes = std::set<std::string>(splited.begin(), splited.end());

	splited = ft::split(p_location.params["allow_methods"]);
	if (!splited[0].empty())
		for (ft::splited_string::const_iterator method = splited.begin(); method != splited.end(); method++)
			location.methods.insert(ft::toLower(*method));

	splited = ft::split(p_location.params["redirect"]);
	location.redir = std::make_pair(0, "");
	if (splited.size() > 1)
	{
		location.redir.second = splited.back();
		splited.pop_back();
		location.redir.first = strtoul(splited.back().c_str(), NULL, 10);
	}

	location.e_is_dir = ft::split(p_location.params["if_request_is_dir"]).back();
	location.size_limit = ft::removePrefixB(ft::split(p_location.params["client_body_size"]).back());

	if (!location.size_limit)
		location.size_limit = std::numeric_limits<size_t>::max();

	this->_bindErrorPages(p_location.params["error_page"], location.error_pages);

	for (std::map<std::string, ParsedEntity>::iterator it = p_location.locations.begin(); it != p_location.locations.end(); it++)
		this->_prepareLocation(it->second, location.locations[it->first]);
}

void	Meta::_prepareServer(ParsedEntity & p_server, Server::Settings & server)
{
	if (p_server.locations.empty())
		throw std::logic_error("server must have at least 1 location");

	ft::splited_string	server_names = ft::split(p_server.params["server_names"]);
	
	if (!server_names[0].empty())
		server.server_names = std::set<std::string>(server_names.begin(), server_names.end());

	ft::splited_string	hosts_ports = ft::split(p_server.params["listen"]);
	ft::key_value_type	host_port;
	ft::key_value_type	port_backlog;
	int					backlog;

	for (ft::splited_string::const_iterator start = hosts_ports.begin(); start != hosts_ports.end(); start++)
	{
		host_port = ft::splitHeader(*start, ":");
		port_backlog = ft::splitHeader(host_port.second, ":");

		if (host_port.first.empty())
			host_port.first = DEF_ADDR;
		if (port_backlog.first.empty())
			port_backlog.first = DEF_PORT;
		
		host_port.second = port_backlog.first;
		backlog = strtol(port_backlog.second.c_str(), NULL, 10);
		if (backlog < 1)
			backlog = DEF_BACKLOG;
			
		server.host_port.insert(std::make_pair(host_port, backlog));
	}
	
	this->_prepareLocation(p_server, server.def_settings);
	this->_checkLocations(server.def_settings, server.def_settings.locations);
}

void	Meta::_checkLocations(Location const & def_loc, Server::locations_type const & locations)
{
	for (Server::locations_type::const_iterator it = locations.begin(); it != locations.end(); it++)
	{
		if (it->second.redir.second.empty() && it->second.root.empty() && def_loc.root.empty())
			throw std::logic_error("location '" + it->first + "': location must have root or redirection");
		
		if (!it->second.locations.empty())
			this->_checkLocations(def_loc, it->second.locations);
	}
}

void	Meta::_readConfLine(std::ifstream & file, std::queue<ParsedEntity> & parsed_servers, size_t & line_counter)
{
	std::string					line;
	ft::splited_string			splited;

	ft::readConfFile(file, line);
	line_counter++;

	if (line.empty())
		return ;

	splited = ft::split(line);

	ft::splited_string::const_iterator	srv = std::find(splited.begin(), splited.end(), "server");

	if (srv == splited.end())
		return ;
	
	if (srv != splited.begin() || splited.size() != 2 || splited[1] != "{")
		THROW_UNEXP_LINE(line_counter, line, ": server keyword line must be kind of 'server {', that is");

	parsed_servers.push(ParsedEntity());
	this->_parseBlock(file, parsed_servers.back(), line_counter);
}

void	Meta::_parseBlock(std::ifstream & conf, ParsedEntity & entity, size_t & line_counter)
{
	std::string			line;
	ft::key_value_type	key_params;

	ft::readConfFile(conf, line);

	while (conf.good() && ft::trim(line) != "}")
	{
		line_counter++;

		if (line.empty())
		{
			ft::readConfFile(conf, line);
			continue ;
		}

		if (line.find(";") != std::string::npos)
			THROW_UNEXP_LINE(line_counter, line, ": ';' not allowed");

		key_params = ft::splitHeader(line, SPACES);

		if (key_params.second.empty())
			THROW_UNEXP_LINE(line_counter, line, ": keywords must be followed by parameters");

		if (std::find(Meta::_keywords.begin(), Meta::_keywords.end(), key_params.first) == Meta::_keywords.end())
			THROW_UNEXP_LINE(line_counter, line, ": unknown keyword");

		if (key_params.first == "location")
		{
			ft::splited_string	splited_loc = ft::split(key_params.second);

			if (splited_loc.size() != 2 || splited_loc[1] != "{")
				THROW_UNEXP_LINE(line_counter, line, ": location keyword line must be kind of 'location [path] {', that is");
			
			this->_parseBlock(conf, entity.locations[splited_loc[0]], line_counter);
		}
		else
			entity.params[key_params.first].append(" " + key_params.second);
		
		ft::readConfFile(conf, line);
	}

	if (!conf.good() && !conf.eof())
		throw std::runtime_error(std::string("config file: ") + strerror(errno));

	if (ft::trim(line) != "}")
		throw std::logic_error("unexpected end of block: missing line with only '}'");
}

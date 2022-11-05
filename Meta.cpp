#include "Meta.hpp"
#include <iostream>

static std::vector<std::string>	_initKeywords(void)
{
	std::ifstream	keywords_file;

	ft::openFile(keywords_file, "keywords");

	std::vector<std::string>	keywords;
	std::string					line;

	while (keywords_file.good() && !keywords_file.eof())
	{
		ft::readConfFile(keywords_file, line);

		if (line.empty())
			continue ;
		
		keywords.push_back(line);
	}
	
	return (keywords);
}

static std::map<int, std::string>	statusesInit(void)
{
	std::ifstream				status_file;

	ft::openFile(status_file, "statuses");

	std::string					line;
	ft::splited_string			splited;
	std::map<int, std::string>	statuses;

	while (!status_file.eof())
	{
		ft::readConfFile(status_file, line);

		if (line.empty())
			continue ;

		splited = ft::split(line, "\t", true);

		if (splited.size() != 2)
			continue ;

		statuses.insert(std::make_pair(atoi(splited[0].c_str()), ft::trim(splited[1])));
	}
	
	return (statuses);
}

const std::vector<std::string>		Meta::_keywords = _initKeywords();
const std::map<int, std::string>	Meta::statuses = statusesInit();

Meta::Meta(void)
	: proto_num(getprotobyname("tcp")->p_proto)
{
};

Meta::Meta(std::ifstream & conf)
	: proto_num(getprotobyname("tcp")->p_proto)
{
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

void	Meta::_prepareLocation(ParsedEntity & p_location, Server::Location & location)
{
	location.root = ft::split(p_location.params["root"]).back();

	location.indexes = ft::split(p_location.params["indexes"]);
	if (location.indexes[0].empty())
		location.indexes.clear();

	location.methods = ft::split(p_location.params["allow_methods"]);
	if (location.methods[0].empty())
		location.methods = Meta::_getAllMethods();

	location.redir = ft::split(p_location.params["redirect"]).back();
	location.e_is_dir = ft::split(p_location.params["if_request_is_dir"]).back();
	location.buf_size = ft::removePrefixB(ft::split(p_location.params["client_body_size"]).back());

	this->_bindErrorPages(p_location.params["error_page"], location.error_pages);

	for (std::map<std::string, ParsedEntity>::iterator it = p_location.locations.begin(); it != p_location.locations.end(); it++)
		this->_prepareLocation(it->second, location.locations[it->first]);
}

void	Meta::_prepareServer(ParsedEntity & p_server, Server::Settings & server)
{
	if (p_server.locations.empty())
		throw std::logic_error("server must have at least 1 location");

	server.server_names = ft::split(p_server.params["server_names"]);

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
			
		server.host_port.push_back(std::make_pair(host_port, backlog));
	}
	
	this->_prepareLocation(p_server, server.def_settings);

	for (std::map<std::string, ParsedEntity>::iterator it = p_server.locations.begin(); it != p_server.locations.end(); it++)
	{
		this->_prepareLocation(it->second, server.locations[it->first]);
		if (server.locations[it->first].root.empty() && server.def_settings.root.empty())
			throw std::logic_error(it->first + ": location must have root");
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
		std::logic_error("line " + ft::num_to_string(line_counter) + ": server keyword line must be kind of 'server {', that is");

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
			throw std::logic_error(std::string("unexpected line" + ft::num_to_string(line_counter) + ": " + line + ": ';' not allowed").c_str());

		key_params = ft::splitHeader(line, SPACES);

		if (key_params.second.empty())
			throw std::logic_error(std::string("unexpected line" + ft::num_to_string(line_counter) + ": " + line + ": keywords must be followed by parameters").c_str());

		if (std::find(Meta::_keywords.begin(), Meta::_keywords.end(), key_params.first) == Meta::_keywords.end())
			throw std::logic_error(std::string("unexpected line" + ft::num_to_string(line_counter) + ": " + line + ": unknown keyword").c_str());

		if (key_params.first == "location")
		{
			ft::splited_string	splited_loc = ft::split(key_params.second);

			if (splited_loc.size() != 2 || splited_loc[1] != "{")
				throw std::logic_error(std::string("line" + ft::num_to_string(line_counter) + ": location keyword line must be kind of 'location /[path] {', that is").c_str());
			
			if (*splited_loc[0].begin() != '/' || *(splited_loc[0].end() - 1) != '/')
				throw std::logic_error(std::string("line" + ft::num_to_string(line_counter) + ": path must start with /").c_str());

			this->_parseBlock(conf, entity.locations[splited_loc[0]], line_counter);
		}
		else
			entity.params[key_params.first].append(key_params.second);
		
		ft::readConfFile(conf, line);
	}

	if (!conf.good() && !conf.eof())
		throw std::runtime_error(strerror(errno));

	if (ft::trim(line) != "}")
		throw std::logic_error("unexpected end of block: missing line with only '}'");
}

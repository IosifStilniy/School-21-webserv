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

const std::vector<std::string>	Meta::_keywords = _initKeywords();

Meta::Meta(void)
{
};

Meta::Meta(std::string const & address, int port): address(address), port(port), proto_num(getprotobyname("tcp")->p_proto), backlog(1)
{
	int	reuse_option = 1;
	int	buf;
	socklen_t len;

	this->listen_sock = socket(PF_INET, SOCK_STREAM, this->proto_num);
	std::memset(&this->server_addr, 0, sizeof(this->server_addr));
	server_addr.sin_addr.s_addr = inet_addr(address.c_str());
	server_addr.sin_family = PF_INET;
	server_addr.sin_port = htons(port);
	setsockopt(this->listen_sock, SOL_SOCKET, SO_REUSEADDR, &reuse_option, sizeof(reuse_option));
	if (bind(listen_sock, reinterpret_cast<sockaddr *>(&server_addr), sizeof(server_addr)))
		throw std::runtime_error(strerror(errno));
	fcntl(this->listen_sock, F_SETFL, O_NONBLOCK);
}

Meta::Meta(std::string const & conf_file)
{
	std::ifstream	conf(conf_file);

	ft::openFile(conf, conf_file);

	size_t						line_counter = 0;
	std::queue<ParsedEntity>	parsed_servers;

	while (conf.good())
		this->_readConfLine(conf, parsed_servers, line_counter);

	if (!conf.eof())
		throw std::runtime_error(std::string(conf_file + ": " + strerror(errno)).c_str());
	
	while (!parsed_servers.empty())
	{
		this->servers.push_back(Server());
		this->_prepareServer(parsed_servers.front(), this->servers.back());
		parsed_servers.pop();
	}
	
}

Meta::~Meta()
{
}

void	Meta::_prepareServer(ParsedEntity & p_server, Server & server)
{
	if (p_server.locations.empty())
		throw std::logic_error("server must have at least 1 location");

	if (!p_server.params["server_name"].empty())
		server.server_name = p_server.params["server_name"][0];

	if (!p_server.params["listen"].empty())
	{
		ft::splited_string	host_port = ft::splitHeader(p_server.params["listen"][0], ":");

		if (!host_port[0].empty())
			server.host = host_port[0];
		if (!host_port[1].empty())
			server.port = htons(strtoul(host_port[1].c_str(), NULL, 10));
	}

	for (std::map<std::string, ParsedEntity>::const_iterator location = p_server.locations.begin(); location != p_server.locations.end(); location++)
	{

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
	ft::splited_string	splited;
	ft::splited_string	params;
	std::string			keyword;

	ft::readConfFile(conf, line);

	while (conf.good() && ft::trim(line) != "}")
	{
		line_counter++;

		if (line.empty())
		{
			ft::readConfFile(conf, line);
			continue ;
		}

		splited = ft::splitHeader(line, SPACES);

		if (splited.size() != 2)
			throw std::logic_error(std::string("unexpected line" + ft::num_to_string(line_counter) + ": " + line).c_str());

		keyword = splited[0];

		if (std::find(Meta::_keywords.begin(), Meta::_keywords.end(), keyword) == Meta::_keywords.end())
			throw std::logic_error(std::string("unexpected line" + ft::num_to_string(line_counter) + ": " + line).c_str());

		params = ft::split(splited[1]);

		if (keyword == "location")
		{
			if (params.size() != 2 || params[1] != "{")
				throw std::logic_error(std::string("unexpected line" + ft::num_to_string(line_counter) + ": " + line).c_str());

			this->_parseBlock(conf, entity.locations[params[0]], line_counter);
		}
		else
			entity.params[keyword] = params;
		
		ft::readConfFile(conf, line);
	}

	if (!conf.good() && !conf.eof())
		throw std::runtime_error(strerror(errno));
}

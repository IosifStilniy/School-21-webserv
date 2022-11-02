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

	std::string			line;
	ft::splited_string	splited;
	size_t				line_counter = 0;

	while (conf.good())
	{
		ft::readConfFile(conf, line);
		line_counter++;

		if (line.empty())
			continue ;

		splited = ft::split(line);

		ft::splited_string::const_iterator	srv = std::find(splited.begin(), splited.end(), "server");

		if (srv == splited.end())
			continue ;
		
		if (srv != splited.begin() || splited.size() != 2 || splited[1] != "{")
			std::logic_error("line " + ft::num_to_string(line_counter) + ": server keyword line must be kind of 'server {', that is");

		this->servers.push_back(Server());
		this->_parseServerBlock(conf, this->servers.back());
	}

	if (!conf.eof())
		throw std::runtime_error(std::string(conf_file + ": " + strerror(errno)).c_str());
}

Meta::~Meta()
{
}

void	Meta::_parseServerBlock(std::ifstream & conf, Server & server, size_t & line_counter)
{
	std::string			line;
	ft::splited_string	splited;
	ft::splited_string	params;
	std::string			keyword;
	Location			def_loc;

	while (conf.good() && ft::trim(line) != "}")
	{
		ft::readConfFile(conf, line);
		line_counter++;

		if (line.empty())
			continue ;

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

			this->_parseLocationBlock(conf, server.locations[params[0]]);
		}
		else if (keyword == "listen")
		{
			ft::splited_string	host_port = ft::split(params[0], ":");

			server.host = host_port[0];
			if (params.size() > 1)
				server.port = host_port[1];
		}
	}

	if (!conf.good() && !conf.eof())
		throw std::runtime_error(strerror(errno));

	if (server.locations.empty())
		throw std::logic_error("server must have at least 1 location");
}

void	Meta::_parseLocationBlock(std::ifstream & conf, Location & location, std::string & line, size_t & line_counter)
{
	while (line.find("}") == std::string::npos)
	{
		ft::readConfFile(conf, line);
	}
}
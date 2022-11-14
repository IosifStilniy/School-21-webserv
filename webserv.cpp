#include <string>
#include <iostream>
#include "Meta.hpp"

int	main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cerr << "usage: webserv [path_to_config_file]" << std::endl;
		return (1);
	}

	std::ifstream	conf;

	ft::openFile(conf, argv[1]);

	std::string	path = std::string(argv[0], strrchr(argv[0], '/'));

	if (chdir(path.c_str()))
	{
		std::cerr << "chdir: " << path + ": " + strerror(errno) << std::endl;
		return (1);
	}

	Meta	meta;

	try
	{
		meta.configurateServers(conf);
	}
	catch(const std::exception& e)
	{
		std::cerr << "error: " << e.what() << std::endl;
		return (1);
	}

	Meta::servers_type::iterator	it = meta.servers.begin();

	try
	{
		for (; it != meta.servers.end(); it++)
			it->second.startListen();
	}
	catch(const std::exception& e)
	{
		std::cerr << "error: " << e.what() << std::endl;
		return (1);
	}
	
	while (1)
		for (it = meta.servers.begin(); it != meta.servers.end(); it++)
			it->second.proceed(500);

	return (0);
}

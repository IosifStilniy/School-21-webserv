#include "webserv.hpp"
#include <string>

int	main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cout << "usage: webserv [path_to_config_file]" << std::endl;
		return (1);
	}

	std::ifstream	conf;

	ft::openFile(conf, argv[1]);

	Meta							meta(conf);
	Meta::servers_type::iterator	it = meta.servers.begin();

	for (; it != meta.servers.end(); it++)
		it->second.startListen();
	
	while (1)
		for (it = meta.servers.begin(); it != meta.servers.end(); it++)
			it->second.proceed(500);
}

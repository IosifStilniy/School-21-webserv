#include "webserv.hpp"
#include <string>

static void	init(std::string const & prog_name)
{
	ft::path = std::string(prog_name.begin(), std::find(prog_name.rbegin(), prog_name.rend(), '/').base());
}

int	main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cerr << "usage: webserv [path_to_config_file]" << std::endl;
		return (1);
	}

	init(argv[0]);

	std::ifstream	conf;

	ft::openFile(conf, argv[1]);

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

	for (; it != meta.servers.end(); it++)
		it->second.startListen();
	
	while (1)
		for (it = meta.servers.begin(); it != meta.servers.end(); it++)
			it->second.proceed(500);

	return (0);
}

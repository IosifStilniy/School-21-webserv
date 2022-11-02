#include "webserv.hpp"
#include <string>

int	main(int argc, char **argv)
{
	Meta		meta(argv[1], std::stoi(argv[2]));
	sockaddr_in	client;
	socklen_t	len;

	listen(meta.listen_sock, 1);

	Polls				polls(meta.listen_sock, POLLIN);
	RequestCollector	req_coll;
	Maintainer			maintainer;
	ResponseHandler		resp_handler;
	int					socket;

	while (1)
	{
		maintainer.proceedRequests(req_coll);

		polls.poll(500);

		socket = polls.getNextSocket();
		if (!polls.getIndex(socket))
		{
			polls.append(accept(meta.listen_sock, reinterpret_cast<sockaddr *>(&client), &len), POLLIN | POLLOUT);
			socket = polls.getNextSocket();
		}

		while (socket)
		{
			if ((polls[socket]->revents & POLLIN) == POLLIN)
				req_coll.collect(socket);
			
			// std::cout << "socket: " << socket << std::endl;

			if (!req_coll[socket].empty())
				req_coll[socket].front().printOptions(req_coll[socket].front().options);

			if ((polls[socket]->revents & POLLOUT) == POLLOUT)
				resp_handler.giveResponse(maintainer[socket], socket);

			socket = polls.getNextSocket();
		}
	}
}

#include "Server.hpp"

Server::Server(void)
	: maintainer(this->settings)
{
}

Server::~Server()
{
}

void	Server::initialize(std::string const & host_port, int proto_num)
{
	this->host_port = ft::splitHeader(host_port, ":");

	this->backlog = 0;
	for (std::vector<Settings>::iterator set_start = this->settings.begin(); set_start != this->settings.end(); set_start++)
	{
		for (hosts_ports_type::const_iterator hp_start = set_start->host_port.begin(); hp_start != set_start->host_port.end(); hp_start++)
			if (hp_start->first == this->host_port)
				this->backlog += hp_start->second;

		set_start->host_port.clear();
	}
	
	if (this->backlog < 1)
		this->backlog = std::numeric_limits<int>::max();

	int			reuse_option = 1;
	sockaddr_in	server_addr;

	this->polls.append(socket(PF_INET, SOCK_STREAM, proto_num), POLLIN);
	setsockopt(this->polls.getListenSocket(), SOL_SOCKET, SO_REUSEADDR, &reuse_option, sizeof(reuse_option));
	fcntl(this->polls.getListenSocket(), F_SETFL, O_NONBLOCK);

	std::memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_addr.s_addr = inet_addr(this->host_port.first.c_str());
	server_addr.sin_family = PF_INET;
	server_addr.sin_port = htons(strtol(this->host_port.second.c_str(), NULL, 10));

	if (bind(this->polls.getListenSocket(), reinterpret_cast<sockaddr *>(&server_addr), sizeof(server_addr)))
		throw std::runtime_error("bind: socket " + ft::num_to_string(this->polls.getListenSocket()) + ": " + strerror(errno));
}

void	Server::startListen(void)
{
	if (listen(this->polls.getListenSocket(), this->backlog))
		throw std::runtime_error("listen: socket " + ft::num_to_string(this->polls.getListenSocket()) + strerror(errno));
}

void	Server::_poll(int timeout)
{
	if (this->polls.poll(timeout) < 0)
		std::cerr << "polls: " + std::string(strerror(errno)) << std::endl;

	std::vector<int>	bad_sockets = this->polls.clear();

	for (std::vector<int>::const_iterator socket = bad_sockets.begin(); socket != bad_sockets.end(); socket++)
	{
		this->req_coll.erase(*socket);
		this->maintainer.erase(*socket);
	}
}

std::string	Server::_formHeader(Response::header_fields & options, int status)
{
	std::string	header;

	header.append("HTTP/1.1 " + ft::num_to_string(status) + " " + Response::statuses[status] + NL);
	for (Response::header_fields::const_iterator start = options.begin(); start != options.end(); start++)
		if (!start->second.empty())
			header.append(start->first + ": " + start->second + NL);

	if (options["Transfer-Encoding"].find("chunked") == std::string::npos && options["Content-Length"].empty())
		header.append("Content-Length: 0" + std::string(NL));

	header.append(NL);

	options.clear();

	return (header);
}

void	Server::_sendPacket(Response::bytes_type & packet, Response & response, int socket)
{
	if (packet.empty())
		return ;
	
	size_t	ret = send(socket, &packet[0], packet.size(), 0);

	if (ret == packet.size())
		return ;
	
	if (ret < 0)
	{
		std::cerr << "send: " << strerror(errno) << std::endl;
		ret = 0;
	}

	response.chunks.push_front(Response::bytes_type(packet.begin() + ret, packet.end()));
}

void	Server::_formPacket(Response & response, Response::bytes_type & packet)
{
	if (!response.options.empty())
	{
		std::string		header = this->_formHeader(response.options, response.status);

		std::cout << header << std::flush;
		packet.insert(packet.end(), header.begin(), header.end());
	}

	while (!response.chunks.empty() && !response.chunks.front().empty())
	{
		Response::bytes_type &	chunk = response.chunks.front();

		if (response.trans_mode == Response::tChunked)
		{
			std::string	num = ft::num_to_string(chunk.size(), 16) + Request::nl;

			packet.insert(packet.end(), num.begin(), num.end());
		}

		packet.insert(packet.end(), chunk.begin(), chunk.end());

		if (response.trans_mode == Response::tChunked)
			packet.insert(packet.end(), Request::nl.begin(), Request::nl.end());

		response.chunks.pop_front();
	}
}

void	Server::_giveResponse(Maintainer::response_queue & resp_queue, int socket)
{
	if (resp_queue.empty())
		return ;

	Response &	response = resp_queue.front();

	if (!response.status)
		return ;
	
	if (response.con_status == Response::cKeep_alive)
	{
		int	keep_alive = 1;

		setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, &keep_alive, sizeof(keep_alive));
		response.con_status = Response::cStd;
	}

	Response::bytes_type	packet;

	this->_formPacket(response, packet);

	if (response.trans_mode == Response::tChunked)
	{
		if (response.chunks.empty())
		{
			this->_sendPacket(packet, response, socket);
			return ;
		}

		std::string	zero_chunk = "0" + Request::eof;

		packet.insert(packet.end(), zero_chunk.begin(), zero_chunk.end());
	}
	
	this->_sendPacket(packet, response, socket);

	if (response.con_status == Response::cClose)
		close(socket);

	if (response.empty())
		resp_queue.pop();
}

void	Server::proceed(int timeout)
{
	int	socket;

	this->maintainer.proceedRequests(req_coll);

	this->_poll(timeout);

	socket = this->polls.getNextSocket();
	if (polls.isListenSocket(socket))
	{
		this->polls.append(accept(this->polls.getListenSocket(), NULL, NULL), POLLIN | POLLOUT);
		socket = this->polls.getNextSocket();
	}

	while (socket)
	{
		if ((this->polls[socket]->revents & POLLOUT) == POLLOUT)
			this->_giveResponse(this->maintainer[socket], socket);

		if ((this->polls[socket]->revents & POLLIN) == POLLIN)
			this->req_coll.collect(socket);
		
		socket = this->polls.getNextSocket();
	}
}

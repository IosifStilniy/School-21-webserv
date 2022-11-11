#include "ResponseHandler.hpp"

ResponseHandler::ResponseHandler(void)
{
}

ResponseHandler::~ResponseHandler()
{
}

std::string	ResponseHandler::_formHeader(header_fields & options, int status)
{
	std::string	header;

	header.append("HTTP/1.1 " + ft::num_to_string(status) + " " + Response::statuses[status] + NL);
	for (header_fields::const_iterator start = options.begin(); start != options.end(); start++)
		if (!start->second.empty())
			header.append(start->first + ": " + start->second + NL);

	if (options["Content-Length"].empty())
		header.append("Content-Length: 0" + std::string(NL));

	header.append(NL);

	options.clear();

	return (header);
}

void	ResponseHandler::giveResponse(Maintainer::response_queue & resp_queue, int socket)
{
	if (resp_queue.empty())
		return ;

	response_type &	response = resp_queue.front();

	if (!response.status)
		return ;
	
	if (response.con_status == Response::cKeep_alive)
	{
		int	keep_alive = 1;

		setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, &keep_alive, sizeof(keep_alive));
		response.con_status = Response::cStd;
	}

	if (!response.options.empty())
	{
		std::string		header = this->_formHeader(response.options, response.status);

		std::cout << header << std::flush;
		send(socket, header.c_str(), header.size(), 0);
	}

	while (!response.chunks.empty() && !response.chunks.front().empty())
	{
		bytes_type &	chunk = response.chunks.front();

		if (response.trans_mode == Response::tChunked)
		{
			std::string	num = ft::num_to_string(chunk.size()) + NL;

			send(socket, &num[0], num.size(), 0);
		}

		send(socket, &chunk[0], chunk.size(), 0);

		resp_queue.front().chunks.pop_front();

		if (response.trans_mode == Response::tChunked)
		{
			send(socket, NL, sizeof(NL), 0);
			break ;
		}
	}
	
	if (resp_queue.front().chunks.empty() || resp_queue.front().chunks.front().empty())
	{
		if (response.trans_mode == Response::tChunked)
			send(socket, NL, sizeof(NL), 0);

		if (response.con_status == Response::cClose)
			close(socket);

		resp_queue.pop();
	}
}

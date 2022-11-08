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
	
	if (response.con_status == Response::keep_alive)
	{
		int	keep_alive = 1;

		setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, &keep_alive, sizeof(keep_alive));
		response.con_status = Response::std;
	}

	if (!response.options.empty())
	{
		std::string		header = this->_formHeader(response.options, response.status);

		send(socket, header.c_str(), header.size(), 0);
	}

	bytes_type &	chunk = response.chunks.front();

	if (!chunk.empty())
	{
		send(socket, &chunk[0], chunk.size(), 0);
		resp_queue.front().chunks.pop();
	}
	
	if (resp_queue.front().chunks.empty() || resp_queue.front().chunks.front().empty())
	{
		if (response.con_status == Response::close)
			close(socket);
		resp_queue.pop();
	}
}

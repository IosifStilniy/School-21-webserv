#include "ResponseHandler.hpp"

static std::map<int, std::string>	statusesInit(void)
{
	std::ifstream				status_file("statuses");
	std::string					line;
	ft::splited_string			splited;
	std::map<int, std::string>	statuses;

	while (!status_file.eof())
	{
		std::getline(status_file, line);

		line = line.substr(0, line.find('#'));

		if (line.empty())
			continue ;

		splited = ft::split(line, "\t", true);

		if (splited.size() != 2)
			continue ;

		statuses.insert(std::make_pair(atoi(splited[0].c_str()), ft::trim(splited[1])));
	}
	
	return (statuses);
}

const std::map<int, std::string>	ResponseHandler::_statuses = statusesInit();

ResponseHandler::ResponseHandler(void)
{
}

ResponseHandler::~ResponseHandler()
{
}

std::string	ResponseHandler::_formHeader(header_fields & options, int status)
{
	std::string	header;

	header.append("HTTP/1.1 " + ft::num_to_string(status) + " " + ResponseHandler::_statuses.at(status));
	for (header_fields::const_iterator start = options.begin(); start != options.end(); start++)
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
		resp_queue.pop();
}

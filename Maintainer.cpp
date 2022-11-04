#include "Maintainer.hpp"

Maintainer::Response::Response(void)
	: chunks(), options(), status(0)
{
}

Maintainer::Response::Response(const Response & src)
{
}

void	Maintainer::Response::readFile(std::string const & file_path)
{
	if (!this->in.is_open())
		this->in.open(file_path, std::ios_base::binary);
	
	if (!this->in.good())
		return ;

	if (this->chunks.empty() || this->chunks.back().size() == sizeof(this->buf))
		this->chunks.push(bytes_type());

	this->in.read(this->buf, sizeof(this->buf));

	size_t	length = this->in.gcount();

	this->end = this->buf + length;
	this->spliter = this->end;

	bytes_type &	chunk = this->chunks.back();

	if (length > sizeof(this->buf) - chunk.size())
		this->spliter = this->buf + sizeof(this->buf) - chunk.size();
	
	chunk.insert(chunk.end(), this->buf, this->spliter);

	if (this->spliter != this->end)
		chunks.push(bytes_type(this->spliter, this->end));

	if (this->in.eof())
		this->in.close();
}

static std::vector<std::string>	methodsNameInit(void)
{
	std::vector<std::string>	methods;

	methods.push_back("get");
	methods.push_back("post");
	methods.push_back("delete");

	return (methods);
}

const std::vector<std::string> Maintainer::_methods_names = methodsNameInit();

Maintainer::Maintainer(void)
{
	this->_methods[0] = &Maintainer::_get;
	this->_methods[1] = &Maintainer::_post;
	this->_methods[2] = &Maintainer::_delete;
}

Maintainer::~Maintainer()
{
}

void	Maintainer::_badResponse(int status, Response & response)
{
	response.status = status;

	if (response.in.is_open())
		response.in.close();
	
	while (!response.chunks.empty())
		response.chunks.pop();
	
	response.readFile("root/" + ft::num_to_string(status) + ".html");
	response.options["Content-Length"] = ft::num_to_string(response.chunks.front().size());
	response.options["Content-Type"] = "text/html";
}

void	Maintainer::_get(request_type & request, Response & response)
{
	if (request.getOnlyValue("content-path").back() == '/')
		response.readFile(request.getOnlyValue("content-path") + "index.html");
	else
		response.readFile(request.getOnlyValue("content-path"));
	
	if (response.status)
		return ;

	response.options["Server"] = "webserv/0.1";

	if (!response.in.good() && !response.in.eof())
	{
		this->_badResponse(404, response);
		return ;
	}

	if (response.in.is_open())
	{
		if (!response.options["Transfer-Encoding"].empty())
			response.options["Transfer-Encoding"].append(",");
		response.options["Transfer-Encoding"].append("chunked");
		response.status = 200;
		return ;
	}

	size_t	length = response.chunks.front().size();

	if (response.chunks.front() != response.chunks.back())
		length += response.chunks.back().size();

	response.options["Content-Length"] = ft::num_to_string(length);
	response.status = 200;

}

void	Maintainer::_post(request_type & request, Response & response)
{

}

void	Maintainer::_delete(request_type & request, Response & response)
{

}

void	Maintainer::_dispatchRequest(request_type & request, Response & response)
{
	int	i = 0;

	for (; i < Maintainer::_methods_names.size(); i++)
		if (Maintainer::_methods_names[i] == ft::toLower(request.getOnlyValue("method")))
			break ;

	PTR_FUNC(i)(request, response);
}

Maintainer::response_queue &	Maintainer::operator[](int socket)
{
	return (this->_sockets[socket]);
}

void	Maintainer::proceedRequests(RequestCollector & requests)
{
	for (RequestCollector::iterator	start = requests.begin(); start != requests.end(); start++)
	{
		const int &			socket = start->first;
		request_queue &		req_queue = start->second;

		if (req_queue.empty() || !req_queue.front().is_ready)
			continue ;

		response_queue &	responses = this->_sockets[socket];

		if (responses.empty())
			responses.push(Response());

		this->_dispatchRequest(req_queue.front(), responses.back());

		if (!responses.back().status)
			continue ;

		responses.push(Response());
		req_queue.pop();
	}
}

Maintainer::iterator	Maintainer::begin()
{
	return (this->_sockets.begin());
}

Maintainer::const_iterator	Maintainer::begin()	const
{
	return (this->_sockets.begin());
}

Maintainer::iterator	Maintainer::end()
{
	return (this->_sockets.end());
}

Maintainer::const_iterator	Maintainer::end()	const
{
	return (this->_sockets.end());
}

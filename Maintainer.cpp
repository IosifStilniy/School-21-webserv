#include "Maintainer.hpp"

static std::vector<std::string>	methodsNameInit(void)
{
	std::vector<std::string>	methods;

	methods.push_back("get");
	methods.push_back("post");
	methods.push_back("delete");

	return (methods);
}

const std::vector<std::string> Maintainer::_methods_names = methodsNameInit();

Maintainer::Response::Response(void)
	: chunks(), options(), is_ready(false)
{
}

void	Maintainer::Response::readFile(std::string const & file_path)
{
	if (!this->in.is_open())
		this->in.open(file_path, std::ios_base::binary);

	if (!this->chunks.size() || this->chunks.back().size() == sizeof(this->buf))
		this->chunks.push(bytes_type());

	bytes_type &	chunk = this->chunks.back();
	
	this->in.read(this->buf, sizeof(this->buf));

	size_t	length = this->in.gcount();
	this->end = this->buf + length;
	this->spliter = this->end;

	if (length > sizeof(this->buf) - chunk.size())
		this->spliter = this->buf + sizeof(this->buf) - chunk.size();
	
	chunk.insert(chunk.end(), this->buf, this->spliter);

	if (this->spliter != this->end)
		chunks.push(bytes_type(this->spliter, this->end));

	if (this->in.eof())
		this->in.close();
}

Maintainer::Maintainer(void)
{
}

Maintainer::~Maintainer()
{
}

void	Maintainer::_get(request_type & request, Response & response)
{
	
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
			if (!Maintainer::_methods_names[i].compare(request.options["method"]))
				break ;
	
	PTR_FUNC(i)(request, response);
}

void	Maintainer::proceedRequests(RequestCollector & requests)
{
	for (RequestCollector::iterator	start = requests.begin(), end = requests.end(); start != end; start++)
	{
		const int &			socket = start->first;
		request_queue &		req_queue = start->second;

		if (!req_queue.size() || !req_queue.front().is_ready)
			continue ;

		response_queue &	responses = this->_sockets[socket];

		if (!responses.size())
			responses.push(Response());

		this->_dispatchRequest(req_queue.front(), responses.back());

		if (!responses.back().is_ready)
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

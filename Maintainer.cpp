#include "Maintainer.hpp"

static std::vector<std::string>	methodsNameInit(void)
{
	std::vector<std::string>	methods;

	methods.push_back("head");
	methods.push_back("get");
	methods.push_back("put");
	methods.push_back("post");
	methods.push_back("delete");

	return (methods);
}

const std::vector<std::string> Maintainer::_methods_names = methodsNameInit();

Maintainer::Maintainer(std::vector<ServerSettings> & settings)
	: _settings(settings)
{
	this->_methods[0] = &Maintainer::_head;
	this->_methods[1] = &Maintainer::_get;
	this->_methods[2] = &Maintainer::_put;
	this->_methods[3] = &Maintainer::_post;
	this->_methods[4] = &Maintainer::_delete;
}

Maintainer::~Maintainer()
{
}

void	Maintainer::_head(Request & request, Response & response)
{
	this->_get(request, response);

	if (!response.status)
		return ;

	response.trans_mode = Response::tStd;
	response.chunks.clear();
}

void	Maintainer::_get(Request & request, Response & response)
{
	if (!response.inited)
	{
		response.inited = true;
		try
		{
			response.checkGetPath();
		}
		catch(const std::exception& e)
		{
			std::cerr << e.what() << '\n';
			return ;
		}
	}

	ssize_t	length;

	try
	{
		length = response.readFile();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		response.badResponse(500);
		return ;
	}
	
	if (response.trans_mode == Response::tChunked && static_cast<size_t>(length) < response.buf_size)
	{
		if (!response.chunks.size() || !response.chunks.back().empty())
			response.chunks.push_back(Response::bytes_type());
			
		return ;
	}

	if (response.status)
		return ;

	if (!response.polls.isGood(response.in) && length < 0)
	{
		response.badResponse(404);
		return ;
	}

	if (response.polls.isGood(response.in))
	{
		if (!request.options["TE"].empty() && request.options["TE"].find("chunked") == request.options["TE"].end())
			return ;

		if (!response.options["Transfer-Encoding"].empty())
			response.options["Transfer-Encoding"].append(", ");
		response.options["Transfer-Encoding"].append("chunked");
		response.trans_mode = Response::tChunked;
		response.status = 200;
		return ;
	}

	response.options["Content-Length"] = ft::num_to_string(response.getContentLength());
	response.status = 200;
}

void	Maintainer::_put(Request & request, Response & response)
{
	if (!response.inited)
	{
		response.inited = true;
		try
		{
			response.checkPutPath();
		}
		catch(const std::exception& e)
		{
			std::cerr << e.what() << '\n';
			return ;
		}
	}

	if (request.tr_state == Request::tChunked && (request.chunks.empty() || request.chunks.front().empty()))
		return ;

	response.writeFile(request);

	if (response.status || response.polls.isGood(response.out))
		return ;
	
	if (!request.empty())
	{
		response.badResponse(500);
		return ;
	}

	close(response.out);
	response.out = -1;

	response.status = 201;
	response.options["Location"] = request.getOnlyValue(CONTENT_PATH);
}

void	Maintainer::_post(Request & request, Response & response)
{
	if (!request.options["Content-Length"].empty())
	{
		this->_put(request, response);
		return ;
	}

	if (response.status)
		return ;
	
	response.status = 200;
}

void	Maintainer::_delete(Request & request, Response & response)
{
	static_cast<void>(request);

	if (response.status)
		return ;

	if (response.mounted_path == response.path_location->second.root)
	{
		response.badResponse(403, response.chooseErrorPageSource());
		return ;
	}

	if (!ft::exist(response.mounted_path))
	{
		response.badResponse(404);
		return ;
	}

	if (std::remove(response.mounted_path.c_str()))
	{
		std::cerr << "remove: " << strerror(errno) << std::endl;
		response.badResponse(500);
		return ;
	}

	response.status = 200;
}

void	Maintainer::_dispatchRequest(Request & request, Response & response)
{
	if (!response.inited)
		response.init(request, this->_settings);
	
	try
	{
		if (!response.cgi.getPath().empty())
		{
			response.inited = true;
			response.cgi.handle(request, response);
			return ;
		}
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		response.badResponse(500);
		return ;
	}

	size_t	i = 0;

	for (; i < Maintainer::_methods_names.size(); i++)
		if (Maintainer::_methods_names[i] == ft::toLower(request.getOnlyValue("method")))
			break ;

	if (i < Maintainer::_methods_names.size())
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

		if (req_queue.empty()
			|| (req_queue.front().options.empty() && (req_queue.front().chunks.empty() || req_queue.front().chunks.front().empty())))
			continue ;

		response_queue &	responses = this->_sockets[socket];

		if (responses.empty())
			responses.push(Response());

		if (!req_queue.front().isFullyReceived() && req_queue.front().isStale())
			responses.back().badResponse(408);
		else if (!req_queue.front().options.empty())
			this->_dispatchRequest(req_queue.front(), responses.back());

		if (!responses.back().status
			|| (responses.back().trans_mode == Response::tChunked
				&& (responses.back().chunks.empty() || !responses.back().chunks.front().empty())))
			continue ;

		responses.push(Response());
		req_queue.pop();
	}
}

void	Maintainer::erase(const int socket)
{
	this->_sockets.erase(socket);
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

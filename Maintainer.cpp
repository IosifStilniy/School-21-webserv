#include "Maintainer.hpp"

Maintainer::Response::Response(void)
	: chunks(), options(), status(0), inited(false), settings(nullptr), path_location(std::make_pair("", nullptr))
{
}

void	Maintainer::Response::readFile(void)
{
	this->readFile(this->mounted_path);
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
	{
		this->in.close();
		this->in.clear();
	}
}

std::string const &	Maintainer::Response::chooseErrorPageSource(void)
{
	if (this->path_location.second->e_is_dir.empty())
		return (this->settings->def_settings.e_is_dir);
	return (this->path_location.second->e_is_dir);
}

std::string const &	Maintainer::Response::chooseErrorPageSource(int status)
{
	if (this->path_location.second->error_pages[status].empty())
		return (this->settings->def_settings.error_pages[status]);
	return (this->path_location.second->error_pages[status]);
}

void	Maintainer::Response::badResponse(int status, std::string error_page)
{
	this->status = status;

	if (this->in.is_open())
		this->in.close();
	
	this->in.clear();
	
	while (!this->chunks.empty())
		this->chunks.pop();

	if (error_page.empty())
		error_page = "def_err.html";

	this->readFile(error_page);

	if (error_page == "def_err.html")
		this->chunks.front() = ft::replaceBytes(this->chunks.front(), std::string("STATUS_CODE"), ft::num_to_string(status));

	this->options["Content-Length"] = ft::num_to_string(this->chunks.front().size());
	this->options["Content-Type"] = "text/html";
}

void	Maintainer::Response::_getSettings(RequestCollector::header_values const & host, std::vector<Server::Settings> & settings_collection)
{
	for (std::vector<Server::Settings>::iterator start = settings_collection.begin(); start != settings_collection.end(); start++)
	{
		if (start->server_names.empty() && host.empty())
			;
		else if (std::find(start->server_names.begin(), start->server_names.end(), host.begin()->first) == start->server_names.end())
			continue ;
		
		this->settings = &*start;
		return ;
	}
}

void	Maintainer::Response::_getLocation(Server::locations_type & locations, std::string const & path)
{
	std::string 		loc_path;
	Server::Location *	founded = nullptr;
	size_t				length = 0;

	for (Server::locations_type::iterator start = locations.begin(); start != locations.end(); start++)
	{
		if (path.find(start->first) != 0 || length > start->first.size())
			continue ;
		
		loc_path = start->first;
		founded = &start->second;
		length = start->first.size();
	}

	if (!founded || founded->locations.empty())
	{
		this->path_location = std::make_pair(loc_path, founded);
		return ;
	}

	this->_getLocation(founded->locations, path);

	if (this->path_location.second)
		return ;
	
	this->path_location = std::make_pair(loc_path, founded);

	std::string	root = founded->root;

	if (root.empty())
		root = this->settings->def_settings.root;

	this->mounted_path = ft::replaceBytes(path, loc_path, root);
}

void	Maintainer::Response::_listIndexes(void)
{
	std::ifstream						check;
	std::vector<std::string> const *	indexes = &this->path_location.second->indexes;

	if (indexes->empty())
		indexes = &this->settings->def_settings.indexes;

	for (std::vector<std::string>::const_iterator index = indexes->begin(); index != indexes->end(); index++)
	{
		if (ft::isDirectory(mounted_path + *index))
			continue ;

		check.open(mounted_path + *index);
		check.close();
		
		if (!check.good())
			continue ;

		this->mounted_path.append(*index);

		return ;
	}

	this->badResponse(404, this->chooseErrorPageSource(404));
}

void	Maintainer::Response::_checkMountedPath()
{
	if (!ft::isDirectory(this->mounted_path))
	{
		std::ifstream	check(this->mounted_path);

		check.close();

		if (check.good())
			return ;
		
		this->badResponse(404, this->chooseErrorPageSource(404));

		return ;
	}

	if (this->path_location.second->indexes.empty() && this->settings->def_settings.indexes.empty())
	{
		this->badResponse(403, this->chooseErrorPageSource());
		return ;
	}

	this->_listIndexes();
}

void	Maintainer::Response::init(request_type & request, std::vector<Server::Settings> & settings_collection)
{
	this->inited = true;

	this->_getSettings(request.options["Host"], settings_collection);
	if (!this->settings)
	{
		this->badResponse(404);
		return ;
	}

	this->_getLocation(this->settings->locations, request.getOnlyValue("content-path"));
	if (!this->path_location.second)
	{
		this->badResponse(404, this->settings->def_settings.error_pages[404]);
		return ;
	}

	if (request.getOnlyValue("method") == "get")
		this->_checkMountedPath();
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

Maintainer::Maintainer(std::vector<Server::Settings> & settings)
	: _settings(settings)
{
	this->_methods[0] = &Maintainer::_get;
	this->_methods[1] = &Maintainer::_post;
	this->_methods[2] = &Maintainer::_delete;
}

Maintainer::~Maintainer()
{
}

void	Maintainer::_get(request_type & request, Response & response)
{
	if (!response.inited)
		response.init(request, this->_settings);

	response.readFile();
	
	if (response.status)
		return ;

	response.options["Server"] = "webserv/0.1";

	if (!response.in.good() && !response.in.eof())
	{
		response.badResponse(404, response.chooseErrorPageSource(404));
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

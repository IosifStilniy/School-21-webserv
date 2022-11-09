#include "Response.hpp"
#include "exceptions.hpp"

static std::pair<const int, std::string>	statusesInit(std::string const & line)
{
	ft::key_value_type	k_v = ft::splitHeader(line, "\t");

	return (std::make_pair(strtoul(k_v.first.c_str(), NULL, 10), ft::trim(k_v.second)));
}

std::vector<std::string>	Response::supported_protocols;
std::vector<std::string>	Response::implemented_methods;
std::map<int, std::string>	Response::statuses;

Response::Response(void)
	: status(0), inited(false), con_status(cStd), trans_mode(tStd), settings(nullptr), path_location(std::make_pair("", nullptr)), polls((int [2]){0, 0}, (int [2]){POLLIN, POLLOUT}, 2), in(polls.polls[0].fd), out(polls.polls[1].fd), buf_size(sizeof(*buf) * BUFSIZE), buf(new byte_type[buf_size])
{
	this->in = -1;
	this->out = -1;

	if (Response::implemented_methods.empty())
		ft::containerazeConfFile(Response::implemented_methods, "info/implemented_methods", &ft::returnLine);
	if (Response::supported_protocols.empty())
		ft::containerazeConfFile(Response::supported_protocols, "info/supported_protocols", &ft::returnLine);
	if (Response::statuses.empty())
		ft::containerazeConfFile(Response::statuses, "info/statuses", &statusesInit);
}

Response::Response(const Response & src)
	: status(0), inited(false), con_status(cStd), trans_mode(tStd), settings(nullptr), path_location(std::make_pair("", nullptr)), polls((int []){0, 0}, (int []){POLLIN, POLLOUT}, 2), in(polls.polls[0].fd), out(polls.polls[1].fd), buf_size(sizeof(*buf) * BUFSIZE), buf(new byte_type[buf_size])
{
	static_cast<void>(src);
	this->in = -1;
	this->out = -1;
}

Response::~Response()
{
	::close(this->in);
	::close(this->out);
	delete [] this->buf;
}

void	Response::readFile(void)
{
	this->readFile(this->mounted_path);
}

void	Response::readFile(std::string const & file_path)
{
	if (this->in == -1)
		this->in = open(file_path.c_str(), O_RDONLY | O_NONBLOCK);
	
	if (this->chunks.empty() || this->chunks.back().size() == this->buf_size)
		this->chunks.push_back(bytes_type());

	this->polls.poll(0);

	if (!this->polls.isGood(this->in))
	{
		::close(this->in);
		this->in = -1;
		return ;
	}

	if (!this->polls.isReady(this->in))
		return ;

	size_t	length = read(this->in, this->buf, this->buf_size);

	if (!length)
	{
		::close(this->in);
		this->in = -1;
		return ;
	}

	this->end = this->buf + length;
	this->spliter = this->end;

	bytes_type &	chunk = this->chunks.back();

	if (length > this->buf_size - chunk.size())
		this->spliter = this->buf + this->buf_size - chunk.size();
	
	chunk.insert(chunk.end(), this->buf, this->spliter);

	if (this->spliter != this->end)
		chunks.push_back(bytes_type(this->spliter, this->end));
}

void	Response::writeFile(Request::chunks_type & chunks)
{
	if (chunks.empty() || chunks.front().empty())
		return ;

	if (this->out == -1)
		this->out = open(this->mounted_path.c_str(), O_WRONLY | O_TRUNC | O_NONBLOCK);

	this->polls.poll(0);

	if (!this->polls.isGood(this->out))
	{
		::close(this->out);
		this->out = -1;
		return ;
	}

	if (!this->polls.isReady(this->out))
		return ;

	while (!chunks.empty() || !chunks.front().empty())
	{
		Request::bytes_type &	chunk = chunks.front();

		write(this->out, &chunk[0], chunk.size());
		chunks.pop();
	}

	close(this->out);
	this->out = -1;
}

std::string const &	Response::chooseErrorPageSource(void)
{
	if (this->path_location.second && this->path_location.second->e_is_dir.empty())
		return (this->path_location.second->e_is_dir);
	return (this->settings->def_settings.e_is_dir);
}

std::string const &	Response::chooseErrorPageSource(int status)
{
	if (this->path_location.second && !this->path_location.second->error_pages[status].empty())
		return (this->path_location.second->error_pages[status]);
	return (this->settings->def_settings.error_pages[status]);
}

void	Response::badResponse(int status, std::string error_page)
{
	this->status = status;

	::close(this->in);
	this->in = -1;
	
	this->chunks.clear();

	if (error_page.empty() && this->settings)
		error_page = this->chooseErrorPageSource(status);

	if (error_page.empty())
		error_page = "def_err.html";

	this->readFile(error_page);

	if (error_page == "def_err.html")
		this->chunks.front() = ft::replaceBytes(this->chunks.front(), std::string("STATUS_CODE"), ft::num_to_string(status) + " " + Response::statuses[status]);

	this->options["Server"] = "webserv/0.1";
	this->options["Connection"] = "close";
	this->options["Content-Length"] = ft::num_to_string(this->chunks.front().size());
	this->options["Content-Type"] = "text/html";
}

void	Response::redirect(void)
{
	this->status = 300;
	this->options["Location"] = this->path_location.second->redir;
	this->options["Server"] = "webserv/0.1";
}

void	Response::_checkSettings(Request & request)
{
	if (std::find(Response::supported_protocols.begin(), Response::supported_protocols.end(), request.getOnlyValue(HTTP_V)) == Response::supported_protocols.end())
	{
		for (std::vector<std::string>::const_iterator start = Response::supported_protocols.begin(); start != Response::supported_protocols.end(); start++)
			this->options["Upgrade"].append(" " + *start);
		
		this->options["Connection"].append(" upgrade");

		this->badResponse(426);
		throw ServerSettingsException(*this, request.getOnlyValue(HTTP_V) + ": unsupported protocol");
	}

	if (std::find(Response::implemented_methods.begin(), Response::implemented_methods.end(), request.getOnlyValue(METHOD)) == Response::implemented_methods.end())
	{
		for (std::vector<std::string>::const_iterator start = Response::implemented_methods.begin(); start != Response::implemented_methods.end(); start++)
			this->options["Allow"].append(" " + *start);

		this->badResponse(501);
		throw MethodException(*this, request.getOnlyValue(METHOD), "unimplemeted method");
	}
}

void	Response::_getSettings(Request & request, std::vector<ServerSettings> & settings_collection)
{
	for (std::vector<ServerSettings>::iterator start = settings_collection.begin(); start != settings_collection.end(); start++)
	{
		if (start->server_names.empty() && request.options["Host"].empty())
			;
		else if (std::find(start->server_names.begin(), start->server_names.end(), request.options["Host"].begin()->first) == start->server_names.end())
			continue ;
		
		this->settings = &*start;
		return ;
	}

	this->settings = &settings_collection.front();

	this->_checkSettings(request);
}

void	Response::_checkLocation(Request & request)
{
	if (!this->path_location.second)
	{
		this->badResponse(404);
		throw LocationException(*this, "undefined");
	}

	std::set<std::string> const *	allowed_methods = &this->path_location.second->methods;

	if (allowed_methods->empty())
		allowed_methods = &this->settings->def_settings.methods;

	if (!allowed_methods->empty() && std::find(allowed_methods->begin(), allowed_methods->end(), request.getOnlyValue(METHOD)) == allowed_methods->end())
	{
		std::set<std::string>::const_iterator	start = allowed_methods->begin();

		for (; start != allowed_methods->end(); start++)
			this->options["Allow"].append(" " + ft::toUpper(*start) + ",");
		
		if (this->options["Allow"].back() == ',')
			this->options["Allow"].pop_back();

		this->badResponse(405);

		throw MethodException(*this, request.getOnlyValue(METHOD), "method not allowed");
	}
}

void	Response::_getLocation(Location::locations_type & locations, Request & request)
{
	std::string 		loc_path;
	Location *			founded = nullptr;
	size_t				length = 0;
	std::string const &	path = request.getOnlyValue(CONTENT_PATH);

	for (Location::locations_type::iterator start = locations.begin(); start != locations.end(); start++)
	{
		if (path.find(start->first) != 0 || length > start->first.size())
			continue ;
		
		loc_path = start->first;
		founded = &start->second;
		length = start->first.size();
	}

	if (!founded)
	{
		this->path_location = std::make_pair(loc_path, founded);
		return ;
	}

	if (!founded->locations.empty())
		this->_getLocation(founded->locations, request);

	if (this->path_location.second)
		return ;
	
	this->path_location = std::make_pair(loc_path, founded);

	std::string *	root = &founded->root;

	if (root->empty())
		root = &this->settings->def_settings.root;

	this->mounted_path = ft::replaceBytesOnce(path, loc_path, *root);

	this->_checkLocation(request);
}

void	Response::_listIndexes(void)
{
	std::ifstream					check;
	std::set<std::string> const *	indexes = &this->path_location.second->indexes;

	if (indexes->empty())
		indexes = &this->settings->def_settings.indexes;

	for (std::set<std::string>::const_iterator index = indexes->begin(); index != indexes->end(); index++)
	{
		if (!ft::exist(mounted_path + *index) || ft::isDirectory(mounted_path + *index))
			continue ;

		this->mounted_path.append(*index);
		return ;
	}

	this->badResponse(404);
	throw PathException(*this, "not found");
}

void	Response::_checkGetPath()
{
	if (!ft::isDirectory(this->mounted_path))
	{
		if (ft::exist(this->mounted_path))
			return ;
		
		this->badResponse(404);
		throw PathException(*this, "not found");
	}

	if (this->path_location.second->indexes.empty() && this->settings->def_settings.indexes.empty())
	{
		this->badResponse(403, this->chooseErrorPageSource());
		throw PathException(*this, "is directory");
	}

	this->_listIndexes();
}

void	Response::_checkPostPath(void)
{
	if (ft::exist(this->mounted_path))
	{
		this->badResponse(409, this->chooseErrorPageSource(409));
		throw PathException(*this, "already exist");
	}

	ft::splited_string	splited = ft::split(this->mounted_path.substr(this->path_location.second->root.size()), "/");

	if (splited[0].empty() || splited.back().back() == '/')
	{
		this->badResponse(403, this->chooseErrorPageSource());
		throw PathException(*this, "filename expected");
	}

	splited.pop_back();

	std::string	path = this->path_location.second->root;

	if (path.empty())
		path = this->settings->def_settings.root;

	for (ft::splited_string::const_iterator dir = splited.begin(); dir != splited.end(); dir++)
	{
		path.append("/" + *dir);

		if (ft::exist(path))
			continue ;

		if (mkdir(path.c_str(), MOD))
		{
			this->badResponse(500);
			throw PathException(*this, strerror(errno));
		}
	}
}

void	Response::init(Request & request, std::vector<ServerSettings> & settings_collection)
{
	this->inited = true;

	this->options["Server"] = "webserv/0.1";

	try
	{
		this->_getSettings(request, settings_collection);
		this->_getLocation(this->settings->def_settings.locations, request);

		if (!this->path_location.second->redir.empty())
		{
			this->redirect();
			return ;
		}

		if (request.getOnlyValue(METHOD) == "post")
			this->_checkPostPath();
		else
			this->_checkGetPath();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return ;
	}

	if (request.getOnlyValue("Connection") == "keep-alive")
		this->con_status = cKeep_alive;
	else if (request.getOnlyValue("Connection") == "close")
		this->con_status = cClose;
}

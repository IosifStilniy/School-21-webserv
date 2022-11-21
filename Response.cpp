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

std::string const	Response::_generated_error_page = std::string("<head><title>STATUS_CODE</title></head>\n")
														+ "<body bgcolor=\"white\">\n"
														+ "<center><h1>STATUS_CODE</h1></center>\n"
														+ "<hr><center>webserv</center>\n"
														+ "</body>";

Response::Response(void)
	: status(0), inited(false), content_length(0), con_status(cStd), trans_mode(tStd), settings(nullptr), path_location(nullptr), polls((int [2]){0, 0}, (int [2]){POLLIN, POLLOUT}, 2), in(polls.polls[0].fd), out(polls.polls[1].fd), buf_size(sizeof(*buf) * BUFSIZE), buf(new byte_type[buf_size])
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
	: status(0), inited(false), content_length(0), con_status(cStd), trans_mode(tStd), settings(nullptr), path_location(nullptr), polls((int [2]){0, 0}, (int [2]){POLLIN, POLLOUT}, 2), in(polls.polls[0].fd), out(polls.polls[1].fd), buf_size(sizeof(*buf) * BUFSIZE), buf(new byte_type[buf_size])
{
	static_cast<void>(src);
	this->in = -1;
	this->out = -1;
}

Response::~Response()
{
	close(this->in);
	close(this->out);
	delete [] this->buf;
}

ssize_t	Response::readFile(void)
{
	return (this->readFile(this->mounted_path));
}

ssize_t	Response::readFile(std::string const & file_path)
{
	if (this->in == -1)
		this->in = open(file_path.c_str(), O_RDONLY | O_NONBLOCK);

	if (this->in == -1)
		throw PathException(404, *this, "not found");

	if (this->polls.poll(0) < 0)
		throw BadResponseException(500, *this, "poll: " + std::string(strerror(errno)));

	if (!this->polls.isGood(this->in))
	{
		::close(this->in);
		this->in = -1;
	}

	if (!this->polls.isReady(this->in))
		return (0);

	ssize_t	length = read(this->in, this->buf, this->buf_size);

	if (length < 0)
		throw BadResponseException(500, *this, "read: " + std::string(strerror(errno)));

	this->content_length -= length;

	if (this->content_length < 0)
		throw BadResponseException(500, *this, this->mounted_path + ": wrong filesize: file must be smaller");

	if (!length)
	{
		if (this->content_length > 0)
			throw BadResponseException(500, *this, this->mounted_path + ": wrong filesize: file must be bigger");

		::close(this->in);
		this->in = -1;

		return (length);
	}
	else if (this->chunks.empty() || !(this->chunks.back().size() < this->buf_size))
		this->chunks.push_back(bytes_type());

	byte_type *	end = this->buf + length;
	byte_type *	spliter = end;

	bytes_type &	chunk = this->chunks.back();

	if (static_cast<size_t>(length) > this->buf_size - chunk.size())
		spliter = this->buf + this->buf_size - chunk.size();
	
	chunk.insert(chunk.end(), this->buf, spliter);

	if (spliter != end)
		chunks.push_back(bytes_type(spliter, end));
	
	if (!(static_cast<size_t>(length) < this->buf_size))
		return (length);
	
	close(this->in);
	this->in = -1;

	return (length);
}

void	Response::writeFile(Request & request)
{
	if (this->out == -1)
		this->out = open(this->mounted_path.c_str(), O_WRONLY | O_TRUNC | O_NONBLOCK | O_CREAT, MOD);

	this->polls.poll(0);

	if (request.chunks.empty() || request.chunks.front().empty())
	{
		if (!request.content_length && request.tr_state == Request::tStd)
		{
			close(this->out);
			this->out = -1;
		}

		return ;
	}

	if (!this->polls.isReady(this->out))
	{
		if (!this->polls.isGood(this->out))
		{
			close(this->out);
			this->out = -1;
		}

		return ;
	}

	Request::bytes_type &	chunk = request.chunks.front();

	ssize_t	ret = write(this->out, &chunk[0], chunk.size());

	if (ret < 0)
		throw BadResponseException(500, *this, "write: " + std::string(strerror(errno)));
	
	if (request.tr_state == Request::tStd)
		this->content_length -= ret;

	if (this->content_length < 0)
		throw BadResponseException(500, *this, "bad content size");

	Request::bytes_type	remain;

	if (static_cast<size_t>(ret) < chunk.size())
		remain.assign(chunk.begin() + ret, chunk.end());

	request.chunks.pop_front();

	if (remain.size())
		request.chunks.push_front(remain);
}

std::string const &	Response::chooseErrorPageSource(void)
{
	if (this->path_location && !this->path_location->second.e_is_dir.empty())
		return (this->path_location->second.e_is_dir);
	return (this->settings->def_settings.e_is_dir);
}

std::string const &	Response::chooseErrorPageSource(int status)
{
	if (this->path_location && !this->path_location->second.error_pages[status].empty())
		return (this->path_location->second.error_pages[status]);
	return (this->settings->def_settings.error_pages[status]);
}

void	Response::badResponse(int status, std::string error_page)
{
	this->inited = true;
	this->status = status;
	this->con_status = cClose;
	this->trans_mode = tStd;

	::close(this->in);
	this->in = -1;

	this->chunks.clear();

	if (error_page.empty() && this->settings)
		error_page = this->chooseErrorPageSource(status);

	if (error_page.empty())
		error_page = "def_err.html";

	this->content_length = ft::getFileSize(error_page);

	try
	{
		this->readFile(error_page);
		while (this->polls.isGood(this->in))
			this->readFile(error_page);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		this->chunks.push_back(bytes_type(Response::_generated_error_page.begin(), Response::_generated_error_page.end()));
	}

	for (chunks_type::const_iterator chunk = ++this->chunks.begin(); chunk != this->chunks.end(); chunk++)
		this->chunks.front().insert(this->chunks.front().end(), chunk->begin(), chunk->end());
	
	this->chunks.erase(++this->chunks.begin(), this->chunks.end());

	if (error_page == "def_err.html")
		this->chunks.front() = ft::replaceBytes(this->chunks.front(), std::string("STATUS_CODE"), ft::num_to_string(status) + " " + Response::statuses[status]);

	this->options["Server"] = "webserv/0.1";
	this->options["Connection"] = "close";
	this->options["Content-Length"] = ft::num_to_string(this->getContentLength());
	this->options["Content-Type"] = "text/html";
}

void	Response::redirect(void)
{
	this->status = this->path_location->second.redir.first;
	this->options["Location"] = this->path_location->second.redir.second;
	this->options["Server"] = "webserv/0.1";
}

void	Response::_checkSettings(Request & request)
{
	if (std::find(Response::supported_protocols.begin(), Response::supported_protocols.end(), request.getOnlyValue(HTTP_V)) == Response::supported_protocols.end())
	{
		for (std::vector<std::string>::const_iterator start = Response::supported_protocols.begin(); start != Response::supported_protocols.end(); start++)
			this->options["Upgrade"].append(" " + *start);
		
		this->options["Connection"].append(" upgrade");

		throw ServerSettingsException(426, *this, request.getOnlyValue(HTTP_V) + ": unsupported protocol");
	}

	if (std::find(Response::implemented_methods.begin(), Response::implemented_methods.end(), request.getOnlyValue(METHOD)) == Response::implemented_methods.end())
	{
		for (std::vector<std::string>::const_iterator start = Response::implemented_methods.begin(); start != Response::implemented_methods.end(); start++)
			this->options["Allow"].append(" " + ft::toUpper(*start) + ",");
		
		if (this->options["Allow"].back() == ',')
			this->options["Allow"].pop_back();

		throw ServerSettingsException(501, *this, request.getOnlyValue(METHOD) + ": unimplemented method");
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
	if (!this->path_location)
	{
		this->badResponse(404);
		throw BadRequestException(404, *this, "location: " + request.getOnlyValue("content-path") + ": undefined");
	}

	std::set<std::string> const *	allowed_methods = &this->path_location->second.methods;

	if (allowed_methods->empty())
		allowed_methods = &this->settings->def_settings.methods;

	if (!allowed_methods->empty() && std::find(allowed_methods->begin(), allowed_methods->end(), request.getOnlyValue(METHOD)) == allowed_methods->end())
	{
		std::set<std::string>::const_iterator	start = allowed_methods->begin();

		for (; start != allowed_methods->end(); start++)
			this->options["Allow"].append(" " + ft::toUpper(*start) + ",");
		
		if (this->options["Allow"].back() == ',')
			this->options["Allow"].pop_back();

		throw MethodException(405, *this, request.getOnlyValue(METHOD), "method not allowed");
	}

	if (strtoul(request.getOnlyValue("Content-Length").c_str(), NULL, 10) > this->path_location->second.size_limit)
		throw SizeLimitException(*this, request.getOnlyValue("Content-Length"));
}

void	Response::_getEndPointLocation(Location::locations_type & locations, std::string const & method)
{
	path_location_type	founded = nullptr;
	size_t				length = 0;

	for (Location::locations_type::iterator start = locations.begin(); start != locations.end(); start++)
	{
		if (start->first.front() == '/' || length > start->first.size()
			|| (!start->second.methods.empty() && start->second.methods.find(method) == start->second.methods.end())
			|| this->mounted_path.find(start->first) == std::string::npos
			|| (this->mounted_path.find(start->first) + start->first.size() != this->mounted_path.size()
				&& this->mounted_path[this->mounted_path.find(start->first) + start->first.size()] != '/'))
			continue ;

		founded = &*start;
		length = start->first.size();
	}

	if (!founded)
		return ;
	
	this->path_location = founded;
}

Response::path_location_type	Response::_getMidPointLocation(Location::locations_type & locations, std::string const & path)
{
	std::string 								loc_path;
	size_t										length = 0;
	std::pair<const std::string, Location> *	founded = nullptr;
	
	for (Location::locations_type::iterator start = locations.begin(); start != locations.end(); start++)
	{
		if (path.find(start->first) != 0 || length > start->first.size())
			continue ;
		
		founded = &*start;
		length = start->first.size();
	}

	if (!founded)
		return (founded);
	
	std::string *	root = &founded->second.root;

	if (root->empty())
		root = &this->settings->def_settings.root;

	this->mounted_path = ft::replaceBytesOnce(path, founded->first, *root);

	return (founded);
}

void	Response::_getLocation(Location::locations_type & locations, Request & request)
{
	std::string const &	path = request.getOnlyValue(CONTENT_PATH);
	path_location_type	founded = this->_getMidPointLocation(locations, path);

	if (!founded)
		return ;

	if (!founded->second.locations.empty())
		this->_getLocation(founded->second.locations, request);

	if (this->path_location)
	{
		if (this->path_location->first.front() != '/')
			return ;
		
		this->_getEndPointLocation(locations, request.getOnlyValue(METHOD));
		return ;
	}

	this->path_location = founded;
	this->_getEndPointLocation(locations, request.getOnlyValue(METHOD));
	this->_checkLocation(request);
}

void	Response::_listIndexes(void)
{
	std::ifstream					check;
	std::set<std::string> const *	indexes = &this->path_location->second.indexes;

	if (indexes->empty())
		indexes = &this->settings->def_settings.indexes;

	for (std::set<std::string>::const_iterator index = indexes->begin(); index != indexes->end(); index++)
	{
		if (!ft::exist(mounted_path + *index) || ft::isDirectory(mounted_path + *index))
			continue ;

		this->mounted_path.append(*index);
		return ;
	}

	throw PathException(404, *this, "not found");
}

void	Response::checkGetPath(void)
{
	this->checkGetPath(this->mounted_path);
}

void	Response::checkGetPath(std::string & path)
{
	if (!ft::isDirectory(path))
	{
		if (ft::exist(path))
			return ;

		throw PathException(404, *this, "not found");
	}

	if (this->path_location->second.indexes.empty() && this->settings->def_settings.indexes.empty())
		throw PathException(403, *this, "is directory", this->chooseErrorPageSource());

	if (path.back() != '/')
		path.append("/");

	this->_listIndexes();
}

void	Response::checkPutPath(void)
{
	ft::splited_string	splited = ft::split(this->mounted_path.substr(this->path_location->second.root.size()), "/");

	if (splited[0].empty() || splited.back().back() == '/')
		throw PathException(403, *this, "filename expected", this->chooseErrorPageSource());

	splited.pop_back();

	std::string	path = this->path_location->second.root;

	if (path.empty())
		path = this->settings->def_settings.root;

	for (ft::splited_string::const_iterator dir = splited.begin(); dir != splited.end(); dir++)
	{
		path.append("/" + *dir);

		if (ft::exist(path))
			continue ;

		if (mkdir(path.c_str(), MOD))
			throw PathException(500, *this, "mkdir: " + std::string(strerror(errno)));
	}
}

size_t	Response::getContentLength(void)
{
	size_t	size = 0;

	for (chunks_type::const_iterator chunk = this->chunks.begin(); chunk != this->chunks.end(); chunk++)
		size += chunk->size();
	
	return (size);
}

void	Response::init(Request & request, std::vector<ServerSettings> & settings_collection)
{
	this->options["Server"] = "webserv/0.1";

	if (!request.is_good)
		throw BadRequestException(400, *this, "request not good");

	this->_getSettings(request, settings_collection);
	this->_getLocation(this->settings->def_settings.locations, request);

	if (!this->path_location->second.redir.second.empty())
	{
		this->redirect();
		return ;
	}

	this->cgi.setPath(this->settings->cgi[this->path_location->first]);

	if (request.getOnlyValue("Connection") == "keep-alive")
		this->con_status = cKeep_alive;
	else if (request.getOnlyValue("Connection") == "close")
		this->con_status = cClose;
}

bool	Response::empty(bool release_req)
{
	return ((this->status && this->trans_mode == tStd && !this->content_length && (release_req || this->chunks.empty()))
			|| (this->trans_mode == tChunked && !this->chunks.empty() && this->chunks.front().empty()));
}

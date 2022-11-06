#include "Response.hpp"

Response::Response(void)
	: chunks(), options(), status(0), inited(false), settings(nullptr), path_location(std::make_pair("", nullptr))
{
}

Response::Response(const Response & src)
{
	static_cast<void>(src);
}

void	Response::readFile(void)
{
	this->readFile(this->mounted_path);
}

void	Response::readFile(std::string const & file_path)
{
	if (!this->in.is_open())
	{
		this->in.clear();
		this->in.open(file_path, std::ios_base::binary);
	}
	
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

	if (this->in.is_open())
		this->in.close();
	
	this->in.clear();
	
	while (!this->chunks.empty())
		this->chunks.pop();

	if (error_page.empty() && this->settings)
		error_page = this->chooseErrorPageSource(status);

	if (error_page.empty())
		error_page = "def_err.html";

	this->readFile(error_page);

	if (error_page == "def_err.html")
		this->chunks.front() = ft::replaceBytes(this->chunks.front(), std::string("STATUS_CODE"), ft::num_to_string(status));

	this->options["Content-Length"] = ft::num_to_string(this->chunks.front().size());
	this->options["Content-Type"] = "text/html";
}

void	Response::_getSettings(RequestCollector::header_values const & host, std::vector<ServerSettings> & settings_collection)
{
	for (std::vector<ServerSettings>::iterator start = settings_collection.begin(); start != settings_collection.end(); start++)
	{
		if (start->server_names.empty() && host.empty())
			;
		else if (std::find(start->server_names.begin(), start->server_names.end(), host.begin()->first) == start->server_names.end())
			continue ;
		
		this->settings = &*start;
		return ;
	}
}

void	Response::_getLocation(Location::locations_type & locations, std::string const & path)
{
	std::string 		loc_path;
	Location *			founded = nullptr;
	size_t				length = 0;

	for (Location::locations_type::iterator start = locations.begin(); start != locations.end(); start++)
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

void	Response::_listIndexes(void)
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

	this->badResponse(404);
}

void	Response::_checkMountedPath()
{
	if (!ft::isDirectory(this->mounted_path))
	{
		std::ifstream	check(this->mounted_path);

		check.close();

		if (check.good())
			return ;
		
		this->badResponse(404);
		return ;
	}

	if (this->path_location.second->indexes.empty() && this->settings->def_settings.indexes.empty())
	{
		this->badResponse(403, this->chooseErrorPageSource());
		return ;
	}

	this->_listIndexes();
}

void	Response::init(Request & request, std::vector<ServerSettings> & settings_collection)
{
	this->inited = true;

	this->_getSettings(request.options["Host"], settings_collection);
	if (!this->settings)
	{
		this->badResponse(404);
		return ;
	}

	if (request.getOnlyValue(HTTP_V) != "HTTP/1" && request.getOnlyValue(HTTP_V) != "HTTP/1.0" && request.getOnlyValue(HTTP_V) != "HTTP/1.1")

	this->_getLocation(this->settings->def_settings.locations, request.getOnlyValue("content-path"));
	if (!this->path_location.second)
	{
		this->badResponse(404);
		return ;
	}

	if (request.getOnlyValue(METHOD) == "get")
		this->_checkMountedPath();
}

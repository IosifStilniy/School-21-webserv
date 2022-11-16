#include "CGI.hpp"
#include "Response.hpp"

CGI::CGI(void)
	: _header_extracted(false), stat_loc(-1), buf(new ByteTypes::byte_type[BUFSIZE]), buf_size(BUFSIZE)
{
}

CGI::~CGI()
{
	delete [] this->buf;
}

void	CGI::setPath(std::map<std::string, std::string> & cgi, std::string const & method)
{
	if (cgi.empty())
		return ;

	this->_path = cgi[method];
	if (this->_path.empty())
		this->_path = cgi[""];
}

std::string::const_iterator	CGI::_getPathEdge(std::string const & path, std::string const & delim)
{
	std::string::const_iterator	spliter = std::search(path.begin(), path.end(), delim.begin(), delim.end());

	while (spliter < path.end())
	{
		if (spliter + delim.size() == path.end() || *(spliter + delim.size()) == '/')
			break ;
		
		spliter += delim.size();
		spliter = std::search(spliter, path.end(), delim.begin(), delim.end());
	}

	if (!(spliter < path.end()))
		throw std::out_of_range("location not found");
	
	return (spliter + delim.size());
}

std::string	CGI::_extractPathInfo(std::string const & requested_path, std::string const & loc_path)
{
	std::string::const_iterator	spliter = this->_getPathEdge(requested_path, loc_path);

	return (std::string(spliter, std::find(spliter, requested_path.end(), '?')));
}

std::string	CGI::_translatePath(std::string const & mounted_path, std::string const & loc_path)
{
	return (std::string(mounted_path.begin(), this->_getPathEdge(mounted_path, loc_path)));
}

std::string	CGI::_extractQueryString(std::string const & requested_path)
{
	std::string::const_iterator	query = std::find(requested_path.begin(), requested_path.end(), '?');

	if (query != requested_path.end())
		query++;

	return (std::string(query, requested_path.end()));
}

ft::splited_string	CGI::_vectorizeEnv(void)
{
	ft::splited_string	vec;

	for (std::map<std::string, std::string>::const_iterator var = this->_env.begin(); var != this->_env.end(); var++)
		vec.push_back(var->first + "=" + var->second);

	return (vec);
}

void	CGI::_setEnv(Request & request, std::string const & mounted_path, std::string const & loc_path)
{
	this->_env["HTTP_ACCEPT"] = request.formOptionLine("Accept");
	this->_env["HTTP_ACCEPT_CHARSET"] = request.formOptionLine("Accept-Charset");
	this->_env["HTTP_ACCEPT_ENCODING"] = request.formOptionLine("Accept-Encoding");
	this->_env["HTTP_ACCEPT_LANGUAGE"] = request.formOptionLine("Accept-Language");
	this->_env["HTTP_CONNECTION"] = request.formOptionLine("Connection");
	this->_env["HTTP_HOST"] = request.formOptionLine("Host");
	this->_env["HTTP_USER_AGENT"] = request.formOptionLine("User-Agent");
	this->_env["GATEWAY_INTERFACE"] = "CGI/1.1";
	this->_env["PATH"] = getenv("PATH");
	// this->_env["PATH_INFO"] = this->_extractPathInfo(request.getOnlyValue(CONTENT_PATH), loc_path);
	this->_env["PATH_INFO"] = request.getOnlyValue(CONTENT_PATH);
	this->_env["PATH_TRANSLATED"] = this->_translatePath(mounted_path, loc_path);
	this->_env["QUERY_STRING"] = this->_extractQueryString(request.getOnlyValue(CONTENT_PATH));
	this->_env["REMOTE_ADDR"];
	this->_env["REMOTE_PORT"];
	this->_env["REQUEST_METHOD"] = ft::toUpper(request.getOnlyValue(METHOD));
	this->_env["REQUEST_URI"] = request.getOnlyValue(CONTENT_PATH);
	this->_env["SCRIPT_FILENAME"] = this->_translatePath(mounted_path, loc_path);
	this->_env["SCRIPT_NAME"] = std::string(request.getOnlyValue(CONTENT_PATH).begin(), this->_getPathEdge(request.getOnlyValue(CONTENT_PATH), loc_path));
	this->_env["SERVER_ADDR"];
	this->_env["SERVER_NAME"] = request.formOptionLine("Host");
	this->_env["SERVER_PORT"];
	this->_env["SERVER_PROTOCOL"] = request.getOnlyValue(HTTP_V);
	this->_env["SERVER_SIGNATURE"];
	this->_env["SERVER_SOFTWARE"] = "webserv/0.1";
	this->_env["CONTENT_TYPE"] = request.formOptionLine("Content-Type");
	this->_env["CONTENT_LENGTH"] = ft::num_to_string(request.getContentLength());
	this->_env["HTTP_COOKIE"];
}

void	CGI::_runChild(int server_to_cgi[2], int cgi_to_server[2])
{
	dup2(server_to_cgi[0], 0);
	dup2(cgi_to_server[1], 1);

	close(server_to_cgi[0]);
	close(server_to_cgi[1]);
	close(cgi_to_server[0]);
	close(cgi_to_server[1]);

	std::list<std::string>	args;

	args.push_back(this->_path);
	args.push_back(this->_env["SCRIPT_FILENAME"]);

	ft::splited_string	vec = this->_vectorizeEnv();

	execve(this->_path.c_str(), ft::containerToArray(args), ft::containerToArray(vec));

	std::cerr << "execve: " + this->_path << ": " << strerror(errno) << std::endl;
	exit(1);
}

void	CGI::_setup(Request & request, Response & response)
{
	this->_setEnv(request, response.mounted_path, response.path_location->first);

	int	server_to_cgi[2];
	int	cgi_to_server[2];

	pipe(server_to_cgi);
	pipe(cgi_to_server);

	this->pid = fork();

	if (!this->pid)
		this->_runChild(server_to_cgi, cgi_to_server);
	
	close(server_to_cgi[0]);
	close(cgi_to_server[1]);

	this->in = cgi_to_server[0];
	this->out = server_to_cgi[1];

	fcntl(this->in, F_SETFL, O_NONBLOCK);
	fcntl(this->out, F_SETFL, O_NONBLOCK);

	this->polls.purge();
	this->polls.append(this->out, POLLOUT);

	waitpid(this->pid, &this->stat_loc, WNOHANG | WUNTRACED);

	if (this->stat_loc > 0)
	{
		this->pid = 0;
		this->stat_loc = -1;
		response.badResponse(500);
		return ;
	}
	
	this->polls.poll();

	if (this->polls.isReady(this->out))
		write(this->out, request.raw_header.c_str(), request.raw_header.size());

	this->polls.append(this->in, POLLIN);
}

void	CGI::_getHeaderFromCGI(Response & response, ByteTypes::bytes_type & chunk)
{
	Response::bytes_type::iterator	eof = std::search(chunk.begin(), chunk.end(), Request::eof.begin(), Request::eof.end());

	if (eof == chunk.end())
		return ;
	
	Response::bytes_type::iterator	repited_req_eof = std::search(eof + 4, chunk.end(), Request::eof.begin(), Request::eof.end());

	if (repited_req_eof == chunk.end())
		return ;
	
	// std::cout << "raw cgi header:" << std::endl;
	// std::cout << std::string(chunk.begin(), eof) << std::endl;

	ft::splited_string	splited = ft::split(std::string(chunk.begin(), eof), "\n");
	ft::key_value_type	field_value;

	for (ft::splited_string::const_iterator field = splited.begin(); field != splited.end(); field++)
	{
		field_value = ft::splitHeader(*field, ":");
		response.options[ft::trim(field_value.first)] = ft::trim(field_value.second);
	}

	repited_req_eof += Request::eof.size();

	if (repited_req_eof <= chunk.end())
		response.chunks.push_back(Response::bytes_type(repited_req_eof, chunk.end()));

	response.chunks.pop_front();

	if ((response.options["Transfer-Encoding"].empty() || response.options["Transfer-Encoding"].find("chunked") != std::string::npos)
		&& response.options["Content-Length"].empty())
	{
		if (response.options["Transfer-Encoding"].find("chunked") == std::string::npos)
		{
			if (!response.options["Transfer-Encoding"].empty())
				response.options["Transfer-Encoding"].append(", ");
			response.options["Transfer-Encoding"].append("chunked");
		}

		response.trans_mode = Response::tChunked;
	}

	this->_header_extracted = true;
}

void	CGI::_setResponseStatus(Response & response)
{
	response.status = strtoul(response.options["Status"].c_str(), NULL, 10);
	response.options["Status"].clear();

	// if (response.options["Transfer-Encoding"].find("chunked") != std::string::npos || response.options["Content-Length"].empty())
	// 	response.trans_mode = Response::tChunked;
	// else if (response.options["Content-Length"].empty() && !response.chunks.empty() && !response.chunks.front().empty())
	// 	response.options["Content-Length"] = ft::num_to_string(response.getContentLength());

	if (response.status)
		return ;
	
	ft::splited_string	splited;
	
	for (Response::header_fields::iterator field = response.options.begin(); field != response.options.end(); field++)
	{
		if (!field->second.empty() || field->first.find("HTTP/1.1 ") == std::string::npos)
			continue ;
		
		splited = ft::split(field->first);

		if (splited.size() != 3)
			continue ;
		
		response.status = strtoul(splited[1].c_str(), NULL, 10);

		if (!response.status)
			continue ;
		
		return ;
	}

	throw std::runtime_error("undefined status from CGI");
}

static void	fillTail(Response::bytes_type & tail, Response::chunks_type const & chunks)
{
	for (Response::chunks_type::const_reverse_iterator chunk = chunks.rbegin(); chunk != chunks.rend(); chunk++)
	{
		for (Response::bytes_type::const_reverse_iterator byte = chunk->rbegin(); byte != chunk->rend(); byte++)
		{
			tail.insert(tail.begin(), *byte);
			if (tail.size() == Request::eof.size())
				return ;
		}
	}
}

bool	CGI::_msgRecieved(Response & response)
{
	if (response.trans_mode != Response::tChunked && response.getContentLength() < strtoul(response.options["Content-Length"].c_str(), NULL, 10))
		return (false);
	
	Response::bytes_type	tail;

	tail.reserve(Request::eof.size());
	fillTail(tail, response.chunks);

	if (tail.size() != Request::eof.size())
		return (false);

	return (std::equal(tail.begin(), tail.end(), Request::eof.begin()));
}

void	CGI::_finishRecieving(Response & response)
{
	try
	{
		if (!response.status)
			this->_setResponseStatus(response);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		response.badResponse(500);
	}

	close(this->in);
	close(this->out);

	if (this->stat_loc < 0)
		kill(this->pid, SIGINT);
	
	this->pid = -1;
	this->stat_loc = -1;
}

void	CGI::handle(Request & request, Response & response)
{
	if ((response.status && response.trans_mode != Response::tChunked) || request.tr_state == Request::tChunked)
		return ;

	if (!this->pid)
		this->_setup(request, response);
	
	if (response.status && response.trans_mode != Response::tChunked)
		return ;

	waitpid(this->pid, &this->stat_loc, WNOHANG | WUNTRACED);

	if (this->stat_loc > 0)
	{
		this->pid = 0;
		this->stat_loc = -1;
		response.badResponse(500);
		return ;
	}
	
	this->polls.poll(0);

	while (!request.chunks.empty() && this->polls.isReady(this->out))
	{
		write(this->out, &request.chunks.front().front(), request.chunks.front().size());
		request.chunks.pop_front();
		time(&this->last_modified);
	}
	
	ByteTypes::byte_type *	end;

	std::time_t	now = time(NULL);

	if (!this->polls.isReady(this->in))
	{
		if (now - this->last_modified > CGI_TIMEOUT)
		{
			if (response.trans_mode == Response::tChunked)
				response.chunks.push_back(Response::bytes_type());
			this->_finishRecieving(response);
		}

		return ;
	}
	
	this->last_modified = now;

	end = this->buf + read(this->in, this->buf, this->buf_size);

	if (response.chunks.empty())
		response.chunks.push_back(Response::bytes_type());

	// if (response.options.size() > 1 && end - buf)
	// 	response.chunks.push_back(Response::bytes_type());

	Response::bytes_type &	chunk = response.chunks.back();

	chunk.insert(chunk.end(), this->buf, end);

	if (!this->_header_extracted)
		this->_getHeaderFromCGI(response, chunk);
	
	if (!this->_header_extracted)
		return ;
	
	try
	{
		if (!response.status && response.trans_mode == Response::tChunked)
			this->_setResponseStatus(response);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		response.badResponse(500);
	}

	if (!this->_msgRecieved(response))
		return ;

	this->_finishRecieving(response);
}

std::string const &	CGI::getPath(void)
{
	return (this->_path);
}
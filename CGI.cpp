#include "CGI.hpp"
#include "Response.hpp"
#include "exceptions.hpp"

static std::string	lowerizeLine(std::string const & line)
{
	return (ft::toLower(line));
}

ft::splited_string	CGI::standart_headers;

CGI::CGI(void)
	: _header_extracted(false), in(-1), out(-1), pid(0), stat_loc(-1), buf(new ByteTypes::byte_type[BUFSIZE]), buf_size(BUFSIZE)
{
	if (!CGI::standart_headers.empty())
	{
		CGI::standart_headers = ft::containerazeConfFile<ft::splited_string>("info/cgi_standart_headers", &lowerizeLine);
		CGI::standart_headers.push_back(CONTENT_PATH);
		CGI::standart_headers.push_back(HTTP_V);
		CGI::standart_headers.push_back(METHOD);
	}
}

CGI::~CGI()
{
	delete [] this->buf;

	this->_clear();
}

void	CGI::setPath(std::string const & cgi)
{
	if (cgi.empty())
		return ;

	this->_path = cgi;
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

std::string	CGI::translatePath(std::string const & mounted_path, std::string const & loc_path)
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
	this->_env["PATH_TRANSLATED"] = this->translatePath(mounted_path, loc_path);
	this->_env["QUERY_STRING"] = this->_extractQueryString(request.getOnlyValue(CONTENT_PATH));
	this->_env["REMOTE_ADDR"];
	this->_env["REMOTE_PORT"];
	this->_env["REQUEST_METHOD"] = ft::toUpper(request.getOnlyValue(METHOD));
	this->_env["REQUEST_URI"] = request.getOnlyValue(CONTENT_PATH);
	this->_env["SCRIPT_FILENAME"] = this->translatePath(mounted_path, loc_path);
	this->_env["SCRIPT_NAME"] = std::string(request.getOnlyValue(CONTENT_PATH).begin(), this->_getPathEdge(request.getOnlyValue(CONTENT_PATH), loc_path));
	this->_env["SERVER_ADDR"];
	this->_env["SERVER_NAME"] = request.formOptionLine("Host");
	this->_env["SERVER_PORT"];
	this->_env["SERVER_PROTOCOL"] = request.getOnlyValue(HTTP_V);
	this->_env["SERVER_SIGNATURE"];
	this->_env["SERVER_SOFTWARE"] = "webserv/0.1";
	this->_env["CONTENT_TYPE"] = request.formOptionLine("Content-Type");
	this->_env["CONTENT_LENGTH"] = ft::num_to_string(request.getContentLength());
	this->_env["HTTP_COOKIE"] = request.formOptionLine("Cookie");

	for (Request::header_fields::const_iterator field = request.options.begin(); field != request.options.end(); field++)
	{
		if (std::find(CGI::standart_headers.begin(), CGI::standart_headers.end(), ft::toLower(field->first)) != CGI::standart_headers.end())
			continue ;
		
		this->_env[ft::replaceBytes(ft::toUpper(field->first), std::string("-"), std::string("_"))] = request.formOptionLine(field->first);
	}
}

void	CGI::_runChild(int server_to_cgi[2], int cgi_to_server[2])
{
	fcntl(server_to_cgi[0], F_SETFL, O_NONBLOCK);
	fcntl(cgi_to_server[1], F_SETFL, O_NONBLOCK);

	dup2(server_to_cgi[0], 0);
	dup2(cgi_to_server[1], 1);

	close(server_to_cgi[0]);
	close(server_to_cgi[1]);
	close(cgi_to_server[0]);
	close(cgi_to_server[1]);

	std::list<std::string>	args;

	if (this->_path != "binary")
		args.push_back(this->_path);
	args.push_back(this->_env["SCRIPT_FILENAME"]);

	ft::splited_string	vec = this->_vectorizeEnv();

	execve(args.front().c_str(), ft::containerToArray(args), ft::containerToArray(vec));

	std::cerr << "execve: " + this->_path << ": " << strerror(errno) << std::endl;
	exit(1);
}

void	CGI::_setup(Request & request, Response & response, Request::bytes_type & packet)
{
	this->eated = 0;
	this->readed = 0;

	this->_setEnv(request, response.mounted_path, response.path_location->first);

	int	server_to_cgi[2];
	int	cgi_to_server[2];

	if (pipe(server_to_cgi) || pipe(cgi_to_server))
		throw CGIException(response, "pipe: " + std::string(strerror(errno)));

	this->pid = fork();

	if (this->pid < 0)
		throw CGIException(response, "fork: " + std::string(strerror(errno)));

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
	this->polls.append(this->in, POLLIN);

	time(&this->last_modified);

	// packet.insert(packet.end(), request.raw_header.begin(), request.raw_header.end());
	static_cast<void>(packet);
}

void	CGI::_getHeaderFromCGI(Response & response, ByteTypes::bytes_type & chunk)
{
	Response::bytes_type::iterator	eof = std::search(chunk.begin(), chunk.end(), Request::eof.begin(), Request::eof.end());

	if (eof == chunk.end())
		return ;
	
	ft::splited_string	splited = ft::split(std::string(chunk.begin(), eof), "\n");
	ft::key_value_type	field_value;

	for (ft::splited_string::const_iterator field = splited.begin(); field != splited.end(); field++)
	{
		field_value = ft::splitHeader(*field, ":");
		response.options[ft::trim(field_value.first)] = ft::trim(field_value.second);
	}

	eof += Request::eof.size();

	Response::bytes_type	remain;

	if (eof <= chunk.end())
		remain.assign(eof, chunk.end());
	
	response.content_length = strtoul(response.options["Content-Length"].c_str(), NULL, 10) - remain.size();

	response.chunks.pop_front();

	if (!remain.empty())
		response.chunks.push_front(remain);

	waitpid(this->pid, &this->stat_loc, WNOHANG | WUNTRACED);

	if (this->stat_loc > 0)
		throw CGIException(response, "exited: " + ft::num_to_string(this->stat_loc));
	else if ((response.options["Transfer-Encoding"].empty() || response.options["Transfer-Encoding"].find("chunked") != std::string::npos)
		&& response.options["Content-Length"].empty() && !remain.empty())
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

	response.status = 200;
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
	if (response.trans_mode != Response::tChunked
		&& response.getContentLength() < strtoul(response.options["Content-Length"].c_str(), NULL, 10))
		return (false);
	
	if ((response.options.find("Content-Length") != response.options.end() && !response.options["Content-Length"].empty()
			&& response.getContentLength() >= strtoul(response.options["Content-Length"].c_str(), NULL, 10)))
		return (true);
	
	Response::bytes_type	tail;

	tail.reserve(Request::eof.size());
	fillTail(tail, response.chunks);

	if (tail.size() != Request::eof.size())
		return (false);
	
	return (std::equal(tail.begin(), tail.end(), Request::eof.begin()));
}

void	CGI::_clear(void)
{
	close(this->in);
	close(this->out);

	this->in = -1;
	this->out = -1;

	waitpid(this->pid, &this->stat_loc, WNOHANG | WUNTRACED);

	if (this->stat_loc < 0 && this->pid > 0)
		kill(this->pid, SIGINT);
	
	waitpid(this->pid, &this->stat_loc, WNOHANG | WUNTRACED);
	
	this->pid = -1;
	this->stat_loc = -1;
}

void	CGI::_finishRecieving(Request & request, Response & response)
{
	this->_printFinishedCGIInfo(request);

	if (!response.status)
		this->_setResponseStatus(response);

	this->_clear();

	if (response.trans_mode == Response::tChunked && !response.chunks.back().empty())
		response.chunks.push_back(Response::bytes_type());
}

void	CGI::_writePacket(Request::chunks_type & chunks, Request::bytes_type & packet, Response & response)
{
	if (packet.empty())
		return ;

	ssize_t	ret = write(this->out, &packet[0], packet.size());
	
	time(&this->last_modified);
	
	if (ret < 0 || static_cast<size_t>(ret) > packet.size())
		throw BadResponseException(500, response, "write: " + std::string(strerror(errno)));

	this->eated += ret;

	if (static_cast<size_t>(ret) == packet.size())
		return ;
	
	chunks.push_front(Request::bytes_type(packet.begin() + ret, packet.end()));
}

void	CGI::_formPacket(Request & request, Request::bytes_type & packet)
{
	while (!request.chunks.empty() && packet.size() < MAX_WRITE_BUF)
	{
		ssize_t	remain = MAX_WRITE_BUF - packet.size();

		if (remain <= 0)
			return ;

		Request::bytes_type &			chunk = request.chunks.front();
		Request::bytes_type::iterator	end = chunk.end();

		if (chunk.size() > static_cast<size_t>(remain))
			end = chunk.begin() + remain;

		packet.insert(packet.end(), chunk.begin(), end);

		Request::bytes_type	remain_bytes(end, chunk.end());

		request.chunks.pop_front();

		if (!remain_bytes.empty())
			request.chunks.push_front(remain_bytes);
	}
}

void	CGI::_readPacket(Response & response)
{
	ssize_t	ret = read(this->in, this->buf, this->buf_size);

	if (ret < 0)
		throw CGIException(response, "read: " + std::string(strerror(errno)));
	
	this->readed += ret;

	if (!ret)
		return ;

	time(&this->last_modified);

	if (this->_header_extracted && response.trans_mode == Response::tStd)
		response.content_length -= ret;
	
	if (this->_header_extracted && response.trans_mode == Response::tStd && response.content_length < 0)
		throw CGIException(response, "bad content length");

	if (response.chunks.empty())
		response.chunks.push_back(Response::bytes_type());

	Response::bytes_type &	chunk = response.chunks.back();

	chunk.insert(chunk.end(), this->buf, this->buf + ret);

	if (!this->_header_extracted)
		this->_getHeaderFromCGI(response, chunk);
}

void	CGI::_printFinishedCGIInfo(Request & request)
{
	std::cerr << "--------------------------------------------------" << std::endl;
	std::cerr << "stat_loc: " << this->stat_loc << std::endl;
	std::cerr << "eated: " << this->eated << std::endl;
	std::cerr << "eated + recieved: " << this->eated + request.getContentLength() << std::endl;
	std::cerr << "readed: " << this->readed << std::endl;
	std::cerr << std::endl;
	std::cerr << "--------------------------------------------------" << std::endl;
}

void	CGI::handle(Request & request, Response & response)
{
	if (this->pid < 0 || (response.status && response.trans_mode != Response::tChunked))
		return ;

	Request::bytes_type	packet;

	if (!this->pid)
		this->_setup(request, response, packet);
	
	if (response.status && response.trans_mode != Response::tChunked)
		return ;

	if (request.getContentLength() + packet.size() > MAX_WRITE_BUF || request.tr_state == Request::tStd)
		this->_formPacket(request, packet);

	if (waitpid(this->pid, &this->stat_loc, WNOHANG | WUNTRACED) < 0)
		throw CGIException(response, "waitpid: " + std::string(strerror(errno)));

	this->polls.poll(0);

	if (!packet.empty() && this->polls.isReady(this->out) && this->stat_loc < 0)
		this->_writePacket(request.chunks, packet, response);
	else if (!packet.empty())
		request.chunks.push_back(Request::bytes_type(packet.begin(), packet.end()));

	std::time_t	now = time(NULL);

	if (!this->polls.isReady(this->in))
	{
		if (now - this->last_modified > CGI_TIMEOUT)
		{
			if (response.trans_mode == Response::tChunked)
				response.chunks.push_back(Response::bytes_type());

			this->_finishRecieving(request, response);
		}

		if (this->stat_loc > 0)
			throw CGIException(response, "exited: " + ft::num_to_string(this->stat_loc));
		
		if (!this->stat_loc)
			this->_finishRecieving(request, response);
		
		return ;
	}
	
	this->last_modified = now;

	this->_readPacket(response);

	if (!this->_header_extracted)
		return ;
	
	if (!response.status && response.trans_mode == Response::tChunked)
		this->_setResponseStatus(response);

	if (this->_msgRecieved(response))
		this->_finishRecieving(request, response);
}

std::string const &	CGI::getPath(void)
{
	return (this->_path);
}
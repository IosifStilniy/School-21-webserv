#include "RequestCollector.hpp"
#include <iostream>

const std::string RequestCollector::_eof(HTTP_EOF);

RequestCollector::Request::Request(void)
	: chunks(), options(), content_length(0), transfer_encoding(), is_ready(false)
{
	chunks.push(bytes_type());
}

void	RequestCollector::Request::setValues(std::string const & fieldname, std::string const & values_string)
{
	ft::splited_string	splited_values = ft::split(values_string, ",");

	if (splited_values.empty())
		return ;

	ft::splited_string	params;
	ft::splited_string	key_value;

	for (ft::splited_string::const_iterator start = splited_values.begin(); start != splited_values.end(); start++)
	{
		params = ft::split(*start, ";");

		header_values_params &	new_value = this->options[fieldname].insert(std::make_pair(ft::trim(params[0]), header_values_params())).first->second;

		for (ft::splited_string::const_iterator start = params.begin() + 1; start != params.end(); start++)
		{
			key_value = ft::split(*start, "=");

			if (key_value.size() != 2)
				continue ;

			new_value.insert(std::make_pair(ft::trim(key_value[0]), ft::trim(key_value[1])));
		}
	}
}

std::string const &	RequestCollector::Request::getOnlyValue(header_fields::iterator field)
{
	return (field->second.rbegin()->first);
}

std::string const &	RequestCollector::Request::getOnlyValue(std::string const & field)
{
	return (this->options[field].rbegin()->first);
}

void	RequestCollector::Request::parseHeader(void)
{
	if (!this->options.empty())
		return ;

	ft::splited_string	splited = ft::split(std::string(this->chunks.front().begin(), this->chunks.front().end()), "\n");
	ft::splited_string	splited_line = ft::split(splited[0]);

	this->setValues("method", splited_line[0]);
	this->setValues("content-path", splited_line[1]);
	this->setValues("http-version", splited_line[2]);

	for (ft::splited_string::iterator start = splited.begin() + 1; start != splited.end(); start++)
	{
		splited_line = ft::splitHeader(*start);
		this->setValues(ft::trim(splited_line[0]), splited_line[1]);
	}

	header_fields::iterator	length = this->options.find("Content-Length");

	if (length != this->options.end())
		this->content_length = strtoul(this->getOnlyValue(length).c_str(), NULL, 10);

	length = this->options.find("Transfer-Encoding");

	if (length != this->options.end())
		this->transfer_encoding = length->second;
	
	if (!this->content_length || this->transfer_encoding.find("chunked") != this->transfer_encoding.end())
		this->is_ready = true;
}

bool	RequestCollector::Request::isFullyReceived(void)
{
	if (this->options.empty())
		return (false);

	if ((!this->content_length
		&& (this->transfer_encoding.empty() || this->transfer_encoding.find("chunked") != this->transfer_encoding.end()))
		|| (this->content_length && this->content_length == this->chunks.front().size())
	)
		return (true);

	return (false);
}

void	RequestCollector::Request::printOptions(header_values_params const & options, int indent)
{
	for (header_values_params::const_iterator start = options.begin(); start != options.end(); start++)
	{
		std::cout.width(indent);
		std::cout << "";
		std::cout << start->first << " = " << start->second << std::endl;
	}
}

RequestCollector::RequestCollector(void)
	: _ref_eof(RequestCollector::_eof)
{
}

RequestCollector::~RequestCollector()
{
}

bool	RequestCollector::_transferEnded(byte_type * & msg_start, size_t dstnc)
{
	static std::string	tail = "";
	bool				result = false;

	if (dstnc < this->_ref_eof.size())
	{
		tail.assign(msg_start, msg_start + dstnc);
		return (false);
	}

	std::string	ref = tail;

	dstnc = this->_ref_eof.size() - tail.size();
	ref.append(msg_start, msg_start + dstnc);
	result = !ref.compare(this->_ref_eof);

	if (result)
		msg_start += ref.size() - tail.size();
	else
		msg_start += 2 - tail.size();

	tail.clear();
	return (result);
}

RequestCollector::byte_type *	RequestCollector::_chunkedTransferHandler(Request & request, byte_type * msg_start, byte_type * msg_end)
{
	byte_type *		eof = this->_getEOF(msg_start, msg_end);
	chunks_type &	chunks = request.chunks;
	static size_t	chunk_size = 0;
	size_t			dstnc;

	while (msg_start < msg_end)
	{
		dstnc = chunk_size;
		if (dstnc > msg_end - msg_start)
			dstnc = msg_end - msg_start;
		chunks.back().insert(chunks.back().end(), msg_start, msg_start + dstnc);
		msg_start += dstnc;
		chunk_size -= dstnc;

		if (chunk_size)
			continue ;

		chunks.push(bytes_type());

		if (_transferEnded(msg_start, msg_end - msg_start))
		{
			chunk_size = 0;
			return (msg_start);
		}
		
		chunk_size = strtoul(std::string(
												msg_start,
												std::search(
														msg_start, msg_end,
														this->_ref_eof.begin(), this->_ref_eof.begin() + 2
												)
											).c_str(), &msg_start, 10);
	}

	return (msg_start);
}

bool	RequestCollector::_isSplitedEOF(bytes_type & chunk, byte_type * & msg_start, byte_type * msg_end)
{
	if (chunk.empty() || this->_ref_eof.find(*msg_start) == std::string::npos
		|| RequestCollector::_eof.find(chunk.back()) == std::string::npos)
		return (false);

	bytes_type::iterator	start = chunk.end();

	for (; start != chunk.begin(); start--)
		if (std::search(this->_ref_eof.begin(), this->_ref_eof.end(), start - 1, chunk.end()) == this->_ref_eof.end())
			break ;

	std::string	end(start, chunk.end());
	size_t		dstnc = this->_ref_eof.size() - end.size();

	if (msg_end - msg_start < dstnc)
		return (false);

	end.append(std::string(msg_start, msg_start + dstnc));

	if (end.compare(this->_ref_eof))
		return (false);

	msg_start += dstnc;
	chunk.erase(start, chunk.end());

	return (true);
}

RequestCollector::byte_type *	RequestCollector::_splitIncomingStream(Request & request, byte_type * msg_start, byte_type * msg_end)
{
	byte_type *		eof = this->_getEOF(msg_start, msg_end);
	chunks_type &	chunks = request.chunks;
	size_t			dstnc;

	while (!request.isFullyReceived() && msg_start < msg_end)
	{
		if (request.options.empty())
		{
			if (this->_isSplitedEOF(chunks.front(), msg_start, msg_end))
			{
				request.parseHeader();
				chunks.front().clear();
				eof = this->_getEOF(msg_start, msg_end);
				continue ;
			}

			chunks.front().insert(chunks.front().end(), msg_start, eof);

			if (eof != msg_end)
			{
				request.parseHeader();
				chunks.front().clear();
			}

			msg_start = eof + this->_ref_eof.size();
			eof = this->_getEOF(msg_start, msg_end);

			continue ;
		}

		if (request.transfer_encoding.find("chunked") != request.transfer_encoding.end())
			return (this->_chunkedTransferHandler(request, msg_start, msg_end));

		dstnc = msg_end - msg_start;
		if (chunks.front().size() + dstnc > request.content_length)
			dstnc = request.content_length - chunks.front().size();
		chunks.front().insert(chunks.front().end(), msg_start, msg_start + dstnc);
		msg_start += dstnc;
	}

	return (msg_start);
}

RequestCollector::request_queue &	RequestCollector::operator[](int socket)
{
	return (this->_sockets[socket]);
}

void	RequestCollector::collect(int socket)
{
	request_queue &	requests = this->_sockets[socket];

	if (requests.empty())
		requests.push(Request());

	Request *	request = &requests.back();
	byte_type *	crsr = this->_buf;
	byte_type *	msg_end = this->_buf + recv(socket, this->_buf, BUFSIZE, 0);

	while (crsr < msg_end)
	{
		crsr = this->_splitIncomingStream(*request, crsr, msg_end);

		if (!request->isFullyReceived())
			continue ;

		requests.push(Request());
		request = &requests.back();
	}
}

RequestCollector::iterator	RequestCollector::begin()
{
	return (this->_sockets.begin());
}

RequestCollector::const_iterator	RequestCollector::begin()	const
{
	return (this->_sockets.begin());
}

RequestCollector::iterator	RequestCollector::end()
{
	return (this->_sockets.end());
}

RequestCollector::const_iterator	RequestCollector::end()	const
{
	return (this->_sockets.end());
}

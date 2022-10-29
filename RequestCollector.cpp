#include "RequestCollector.hpp"
#include <iostream>

std::string RequestCollector::_eof(HTTP_EOF);

RequestCollector::Request::Request(void)
	: bytes(), options(), content_length(0), transfer_encoding(), is_ready(false)
{
}

void	RequestCollector::Request::parseHeader(void)
{
	if (this->options.size())
		return ;

	ft::splited_string	splited = ft::split(std::string(this->bytes.begin(), this->bytes.end()), "\n");
	ft::splited_string	splited_line = ft::split(splited[0]);

	this->options["method"] = splited_line[0];
	this->options["content-path"] = splited_line[1];
	this->options["http-version"] = splited_line[2];

	for (ft::splited_string::iterator start = splited.begin() + 1, end = splited.end(); start != end; start++)
	{
		splited_line = ft::splitHeader(*start);
		this->options[ft::trim(splited_line[0])] = ft::trim(splited_line[1]);
	}

	header_fields::iterator	length = this->options.find("Content-Length");

	if (length != this->options.end())
		this->content_length = std::stoul(length->second);

	length = this->options.find("Transfer-Encoding");

	if (length != this->options.end())
		this->transfer_encoding = length->second;
	
	if (!this->content_length && !this->transfer_encoding.size())
		this->is_ready = true;
}

bool	RequestCollector::Request::isFullyReceived(void)
{
	if (!this->options.size())
		return (false);

	if (
		this->is_ready
		|| (!this->content_length && !this->transfer_encoding.size())
		|| (this->content_length && this->content_length == this->bytes.size())
	)
		return (true);

	return (false);
}

RequestCollector::RequestCollector(void)
{
}

RequestCollector::~RequestCollector()
{
}

bool	RequestCollector::_transferEnded(byte_type ** msg_start, size_t dstnc)
{
	static std::string	tail = "";
	bool				result = false;

	if (dstnc < RequestCollector::_eof.size())
	{
		tail.assign(*msg_start, *msg_start + dstnc);
		return (false);
	}

	std::string	ref = tail;

	dstnc = RequestCollector::_eof.size() - tail.size();
	ref.append(*msg_start, *msg_start + dstnc);
	result = !ref.compare(RequestCollector::_eof);

	if (result)
		*msg_start += ref.size() - tail.size();
	else
		*msg_start += 2 - tail.size();

	tail.clear();
	return (result);
}

RequestCollector::byte_type *	RequestCollector::_chunkedTransferHandler(Request & request, byte_type * msg_start, byte_type * msg_end)
{
	byte_type *		eof = RequestCollector::_getEOF(msg_start, msg_end);
	bytes_type &	bytes = request.bytes;
	static size_t	chunk_size = 0;
	size_t			dstnc;

	while (msg_start < msg_end)
	{
		dstnc = chunk_size;
		if (dstnc > msg_end - msg_start)
			dstnc = msg_end - msg_start;
		bytes.insert(bytes.end(), msg_start, msg_start + dstnc);
		msg_start += dstnc;
		chunk_size -= dstnc;

		if (chunk_size)
			continue ;

		if (_transferEnded(&msg_start, msg_end - msg_start))
			return (msg_start + RequestCollector::_eof.size());
	}

	return (msg_start);
}

RequestCollector::byte_type *	RequestCollector::_splitIncomingStream(Request & request, byte_type * msg_start, byte_type * msg_end)
{
	byte_type *		eof = RequestCollector::_getEOF(msg_start, msg_end);
	bytes_type &	bytes = request.bytes;
	size_t			dstnc;

	while (!request.isFullyReceived() && msg_start < msg_end)
	{
		if (!request.options.size())
		{
			bytes.insert(bytes.end(), msg_start, eof);

			if (eof != msg_end)
			{
				request.parseHeader();
				bytes.clear();
			}

			msg_start = eof + RequestCollector::_eof.size();
			eof = RequestCollector::_getEOF(this->_buf, msg_end);

			continue ;
		}

		if (request.transfer_encoding.find("chunked") != std::string::npos)
			return (this->_chunkedTransferHandler(request, msg_start, msg_end));

		dstnc = msg_end - msg_start;
		if (bytes.size() + dstnc > request.content_length)
			dstnc = request.content_length - bytes.size();
		bytes.insert(bytes.end(), msg_start, msg_start + dstnc);
		msg_start += dstnc;
	}

	return (msg_start);
}

RequestCollector::Request &	RequestCollector::operator[](int socket)
{
	return (this->_sockets[socket].front());
}

void	RequestCollector::collect(int socket)
{
	request_queue &	requests = this->_sockets[socket];

	if (!requests.size())
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

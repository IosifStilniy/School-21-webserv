#include "RequestCollector.hpp"
#include <iostream>

RequestCollector::RequestCollector(void)
	: _buf(new byte_type[BUFSIZE])
{
}

RequestCollector::~RequestCollector()
{
	delete [] this->_buf;
}

bool	RequestCollector::_isSplitedEOF(bytes_type & chunk, byte_type * & msg_start, byte_type * msg_end)
{
	if (chunk.empty() || Request::eof.find(*msg_start) == std::string::npos
		|| Request::eof.find(chunk.back()) == std::string::npos)
		return (false);

	bytes_type::iterator	start = chunk.end();

	for (; start != chunk.begin(); start--)
		if (std::search(Request::eof.begin(), Request::eof.end(), start - 1, chunk.end()) == Request::eof.end())
			break ;

	std::string	end(start, chunk.end());
	size_t		dstnc = Request::eof.size() - end.size();

	if (static_cast<size_t>(msg_end - msg_start) < dstnc)
		return (false);

	end.append(std::string(msg_start, msg_start + dstnc));

	if (end != Request::eof)
		return (false);

	msg_start += dstnc;
	chunk.erase(start, chunk.end());

	return (true);
}

RequestCollector::byte_type *	RequestCollector::_readHeader(Request & request, byte_type * msg_start, byte_type * msg_end)
{
	byte_type *		eof = std::search(msg_start, msg_end, Request::eof.begin(), Request::eof.end());
	chunks_type &	chunks = request.chunks;

	if (this->_isSplitedEOF(chunks.back(), msg_start, msg_end))
	{
		request.parseHeader();
		chunks.pop_front();

		return (msg_start);
	}

	chunks.back().insert(chunks.back().end(), msg_start, eof);

	if (eof != msg_end)
	{
		request.parseHeader();
		chunks.pop_front();
	}

	msg_start = eof + Request::eof.size();

	return (msg_start);
}

RequestCollector::byte_type *	RequestCollector::_readBody(Request & request, byte_type * msg_start, byte_type * msg_end)
{
	if (!request.content_length && request.tr_state == Request::tChunked)
		msg_start = request.getChunkSize(msg_start, msg_end);

	if (!request.content_length)
		return (msg_start);
	
	request.chunks.push_back(bytes_type());

	bytes_type &	chunk = request.chunks.back();

	size_t	dstnc = msg_end - msg_start;

	if (dstnc > request.content_length)
		dstnc = request.content_length;

	chunk.insert(chunk.end(), msg_start, msg_start + dstnc);
	msg_start += dstnc;
	request.content_length -= dstnc;

	if (!request.content_length && request.tr_state == Request::tChunked)
		msg_start += 2;

	return (msg_start);
}

RequestCollector::byte_type *	RequestCollector::_splitIncomingStream(Request & request, byte_type * msg_start, byte_type * msg_end)
{
	while (request.options.empty() && msg_start < msg_end)
		msg_start = this->_readHeader(request, msg_start, msg_end);
	
	while (!request.options.empty() && !request.isFullyReceived() && msg_start < msg_end)
		msg_start = this->_readBody(request, msg_start, msg_end);
	
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

	if (msg_end < crsr)
		throw std::runtime_error("recv: " + std::string(strerror(errno)));

	try
	{
		while (crsr < msg_end)
		{
			crsr = this->_splitIncomingStream(*request, crsr, msg_end);

			time(&request->last_modified);

			if (!request->isFullyReceived())
				continue ;

			requests.push(Request());
			request = &requests.back();
		}
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		request->is_good = false;
	}
}

void	RequestCollector::erase(const int socket)
{
	this->_sockets.erase(socket);
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

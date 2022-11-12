#include "RequestCollector.hpp"
#include <iostream>

const std::string RequestCollector::_eof(HTTP_EOF);
const std::string RequestCollector::_nl("\r\n");

RequestCollector::RequestCollector(void)
	: _buf(new byte_type[BUFSIZE]), _ref_eof(RequestCollector::_eof)
{
}

RequestCollector::~RequestCollector()
{
	delete [] this->_buf;
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

	if (static_cast<size_t>(msg_end - msg_start) < dstnc)
		return (false);

	end.append(std::string(msg_start, msg_start + dstnc));

	if (end != this->_ref_eof)
		return (false);

	msg_start += dstnc;
	chunk.erase(start, chunk.end());

	return (true);
}

RequestCollector::byte_type *	RequestCollector::_readHeader(Request & request, byte_type * msg_start, byte_type * msg_end)
{
	byte_type *		eof = this->_getEOF(msg_start, msg_end);
	chunks_type &	chunks = request.chunks;

	if (this->_isSplitedEOF(chunks.back(), msg_start, msg_end))
	{
		request.parseHeader();
		chunks.pop();
		return (msg_start);
	}

	chunks.back().insert(chunks.back().end(), msg_start, eof);

	if (eof != msg_end)
	{
		std::cout << "raw:" << std::endl;
		std::cout << std::string(request.chunks.front().begin(), request.chunks.front().end()) << std::endl;
		request.parseHeader();
		chunks.pop();
	}

	msg_start = eof + this->_ref_eof.size();

	return (msg_start);
}

RequestCollector::byte_type *	RequestCollector::_getChunkSize(Request & request, byte_type * msg_start, byte_type * msg_end)
{
	static std::string	tail;
	byte_type *			spliter = std::search(msg_start, msg_end, RequestCollector::_nl.begin(), RequestCollector::_nl.end());

	spliter += static_cast<size_t>(msg_end - spliter) > RequestCollector::_nl.size() ? RequestCollector::_nl.size() : msg_end - spliter;

	tail.append(msg_start, spliter);
	msg_start = spliter;

	if (tail.substr(0, RequestCollector::_eof.size()).size() == RequestCollector::_eof.size())
	{
		request.content_length = 0;
		request.tr_state = Request::tStd;
		return (msg_start);
	}

	if (std::isdigit(tail.back()))
		return (msg_start);

	request.content_length = strtoul(tail.c_str(), NULL, 10);
	tail.clear();

	return (msg_start);
}

RequestCollector::byte_type *	RequestCollector::_readBody(Request & request, byte_type * msg_start, byte_type * msg_end)
{
	if (!request.content_length && request.tr_state == Request::tChunked)
		msg_start = this->_getChunkSize(request, msg_start, msg_end);

	if (!request.content_length)
		return (msg_start);
	
	request.chunks.push(bytes_type());

	bytes_type &	chunk = request.chunks.back();

	size_t	dstnc = msg_end - msg_start;

	if (dstnc > request.content_length)
		dstnc = request.content_length;

	chunk.insert(chunk.end(), msg_start, msg_start + dstnc);
	msg_start += dstnc;
	request.content_length -= dstnc;

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

	while (crsr < msg_end)
	{
		crsr = this->_splitIncomingStream(*request, crsr, msg_end);

		if (!request->isFullyReceived())
			continue ;

		requests.push(Request());
		request = &requests.back();
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

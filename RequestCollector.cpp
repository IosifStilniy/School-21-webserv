#include "RequestCollector.hpp"
#include <iostream>

const std::string RequestCollector::_eof(HTTP_EOF);

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
	result = (ref == this->_ref_eof);

	if (result)
		msg_start += ref.size() - tail.size();
	else
		msg_start += 2 - tail.size();

	tail.clear();
	return (result);
}

RequestCollector::byte_type *	RequestCollector::_chunkedTransferHandler(Request & request, byte_type * msg_start, byte_type * msg_end)
{
	// byte_type *		eof = this->_getEOF(msg_start, msg_end);
	chunks_type &	chunks = request.chunks;
	static size_t	chunk_size = 0;
	size_t			dstnc;

	while (msg_start < msg_end)
	{
		dstnc = chunk_size;
		if (dstnc > static_cast<size_t>(msg_end - msg_start))
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

	if (static_cast<size_t>(msg_end - msg_start) < dstnc)
		return (false);

	end.append(std::string(msg_start, msg_start + dstnc));

	if (end != this->_ref_eof)
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

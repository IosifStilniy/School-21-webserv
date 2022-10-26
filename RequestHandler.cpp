#include "RequestHandler.hpp"

std::string RequestHandler::_eof(HTTP_EOF);

RequestHandler::Request::Request(void)
	: bytes(), msg_expacted(false)
{
}

RequestHandler::Request::~Request()
{
}

RequestHandler::RequestHandler(void)
{
}

RequestHandler::~RequestHandler()
{
}

bool	RequestHandler::_isHeader(std::string request_line)
{

}

bool	RequestHandler::_handle(RequestHandler::request_queue & requests)
{
	Request &	request = requests.front();
	bool		is_header = this->_isHeader(std::string(request.bytes.begin(), std::find(request.bytes.begin(), request.bytes.end(), '\n') + 1));
	bool		msg_expected = false;

	// if (!is_header && !request.msg_expected)
	// 	response_err();
	if (is_header)
		msg_expected = _headerHandler(std::string(request.bytes.begin(), request.bytes.end());
	else
		_msgHandler(request.bytes);

	requests.pop();
	return (false);
}

void	RequestHandler::collectAndHandle(int socket)
{
	this->_msg_end = this->_buf + recv(socket, this->_buf, BUFSIZE, 0);
	this->_msg_eof = RequestHandler::_getEOF(this->_buf, this->_msg_end);

	request_queue &	requests = this->_sockets[socket];

	if (!requests.size())
		requests.push(Request());

	bytes_type &	bytes = requests.back().bytes;

	bytes.insert(bytes.end(), this->_buf, this->_msg_eof);

	if (this->_msg_eof == this->_msg_end)
		return ;

	requests.push(
		Request(
			this->_msg_eof + RequestHandler::_eof.size(),
			this->_msg_end,
			1 /* _handle(requests) */
		)
	);
}

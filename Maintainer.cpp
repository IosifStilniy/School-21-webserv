#include "Maintainer.hpp"

Maintainer::Response::Response(void)
	: chunks(), options(), is_ready(false)
{
}

Maintainer::Maintainer(void)
{
}

Maintainer::~Maintainer()
{
}

void	Maintainer::_maintainRequest(request_type & request, Response & response)
{

}

void	Maintainer::proceedRequests(RequestCollector & requests)
{
	for (RequestCollector::iterator	start = requests.begin(), end = requests.end(); start != end; start++)
	{
		request_queue &		req_queue = start->second;

		if (!req_queue.size() || !req_queue.front().is_ready)
			continue ;

		response_queue &	responses = this->_sockets[start->first];

		if (responses.size() && responses.back().is_ready)
		{
			responses.push(Response());
			req_queue.pop();
		}
		else if (!responses.size())
			responses.push(Response());

		if (!req_queue.size())
			continue ;

		this->_maintainRequest(req_queue.front(), responses.back());
	}
}

Maintainer::iterator	Maintainer::begin()
{
	return (this->_sockets.begin());
}

Maintainer::const_iterator	Maintainer::begin()	const
{
	return (this->_sockets.begin());
}

Maintainer::iterator	Maintainer::end()
{
	return (this->_sockets.end());
}

Maintainer::const_iterator	Maintainer::end()	const
{
	return (this->_sockets.end());
}

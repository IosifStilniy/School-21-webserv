#ifndef MAINTAINER_HPP
# define MAINTAINER_HPP

# include <vector>
# include <map>
# include <string>
# include <queue>

# include "RequestCollector.hpp"

class Maintainer
{
	public:
		typedef unsigned char						byte_type;
		typedef	std::vector<byte_type>				bytes_type;
		typedef std::queue<bytes_type>				chunks_type;
		typedef bytes_type::const_iterator			bytes_iterator;
		typedef std::map<std::string, std::string>	header_fields;

		typedef	RequestCollector::request_queue		request_queue;
		typedef	RequestCollector::Request			request_type;

		struct Response
		{
			chunks_type		chunks;
			header_fields	options;
			bool			is_ready;

			Response(void);
		};

		typedef std::queue<Response>			response_queue;
		typedef	std::map<int, response_queue>	socket_map;

		typedef socket_map::iterator			iterator;
		typedef socket_map::const_iterator		const_iterator;

	private:
		socket_map	_sockets;

		void	_maintainRequest(request_type & request, Response & response);

	public:
		Maintainer(void);
		~Maintainer();

		void	proceedRequests(RequestCollector & requests);

		iterator	begin();
		const_iterator	begin()	const;
		iterator	end();
		const_iterator	end()	const;
};

#endif

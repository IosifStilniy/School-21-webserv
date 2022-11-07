#ifndef MAINTAINER_HPP
# define MAINTAINER_HPP

# include <vector>
# include <map>
# include <string>
# include <queue>
# include <fstream>

# include "utils.hpp"

# include "Response.hpp"
# include "RequestCollector.hpp"

# define PTR_FUNC(i) ((this->*(this->_methods[i])))

class Maintainer
{
	public:
		typedef	RequestCollector::request_queue		request_queue;
		typedef	Request								request_type;
		typedef std::queue<Response>				response_queue;
		typedef	std::map<int, response_queue>		socket_map;
		typedef socket_map::iterator				iterator;
		typedef socket_map::const_iterator			const_iterator;

	private:
		socket_map						_sockets;
		std::vector<ServerSettings> &	_settings;

		Maintainer(void);

		static const std::vector<std::string>	_methods_names;

		void	_dispatchRequest(request_type & request, Response & response);
		
		void	_get(request_type & request, Response & response);
		// void	_post(request_type & request, Response & response);
		// void	_delete(request_type & request, Response & response);

		void	(Maintainer::*_methods[3])(request_type & request, Response & response);

	public:
		Maintainer(std::vector<ServerSettings> & settings);
		~Maintainer();

		response_queue &	operator[](int socket);

		void	proceedRequests(RequestCollector & requests);
		void	erase(const int socket);

		iterator		begin();
		const_iterator	begin()	const;
		iterator		end();
		const_iterator	end()	const;
};

#endif

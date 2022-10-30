#ifndef MAINTAINER_HPP
# define MAINTAINER_HPP

# include <vector>
# include <map>
# include <string>
# include <queue>
# include <fstream>

# include "utils.hpp"

# include "RequestCollector.hpp"

# define PTR_FUNC(i) ((this->*(this->_methods[i])))

class Maintainer
{
	public:
		typedef char								byte_type;
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

			std::ifstream	in;
			std::ofstream	out;
			byte_type		buf[BUFSIZE];
			byte_type *		spliter;
			byte_type *		end;

			void	readFile(std::string const & filepath);

			Response(void);
		};

		typedef std::queue<Response>			response_queue;
		typedef	std::map<int, response_queue>	socket_map;

		typedef socket_map::iterator			iterator;
		typedef socket_map::const_iterator		const_iterator;

	private:
		socket_map	_sockets;

		static const std::vector<std::string>	_methods_names;

		void	_readFile(std::string const & file_path, Response & response);

		void	_dispatchRequest(request_type & request, Response & response);
		void	_get(request_type & request, Response & response);
		void	_post(request_type & request, Response & response);
		void	_delete(request_type & request, Response & response);

		void	(Maintainer::*_methods[3])(request_type & request, Response & response);

	public:
		Maintainer(void);
		~Maintainer();

		void	proceedRequests(RequestCollector & requests);

		iterator		begin();
		const_iterator	begin()	const;
		iterator		end();
		const_iterator	end()	const;
};

#endif

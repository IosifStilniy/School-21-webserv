#ifndef MAINTAINER_HPP
# define MAINTAINER_HPP

# include <vector>
# include <map>
# include <string>
# include <queue>
# include <fstream>

# include "utils.hpp"

# include "RequestCollector.hpp"
# include "Server.hpp"

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

		typedef	std::pair<std::string, Server::Location *>	path_location_type;

		struct Response
		{
			chunks_type		chunks;
			header_fields	options;
			int				status;
			bool			inited;

			Server::Settings *	settings;
			path_location_type	path_location;
			std::string			mounted_path;

			std::ifstream	in;
			std::ofstream	out;
			byte_type		buf[BUFSIZE];
			byte_type *		spliter;
			byte_type *		end;

			Response(void);

			private:
				void	_getSettings(RequestCollector::header_values const & host, std::vector<Server::Settings> & settings);
				void	_getLocation(Server::locations_type & locations, std::string const & path);
				void	_checkMountedPath(void);
				void	_listIndexes(void);

			public:
				void				readFile(void);
				void				readFile(std::string const & filepath);
				void				init(request_type & request, std::vector<Server::Settings> & settings_collection);
				void				badResponse(int status, std::string error_page = "");
				std::string const &	chooseErrorPageSource(void);
				std::string const &	chooseErrorPageSource(int status);
		};

		typedef std::queue<Response>			response_queue;
		typedef	std::map<int, response_queue>	socket_map;

		typedef socket_map::iterator			iterator;
		typedef socket_map::const_iterator		const_iterator;

	private:
		typedef	std::map<std::string, std::string>	locations_type;

	private:
		socket_map						_sockets;
		locations_type					_locations;
		std::vector<Server::Settings> &	_settings;

		Maintainer(void);

		static const std::vector<std::string>	_methods_names;

		void	_readFile(std::string const & file_path, Response & response);

		void	_dispatchRequest(request_type & request, Response & response);
		
		void	_get(request_type & request, Response & response);
		void	_post(request_type & request, Response & response);
		void	_delete(request_type & request, Response & response);

		void	(Maintainer::*_methods[3])(request_type & request, Response & response);

	public:
		Maintainer(std::vector<Server::Settings> & settings);
		~Maintainer();

		response_queue &	operator[](int socket);

		void	proceedRequests(RequestCollector & requests);

		iterator		begin();
		const_iterator	begin()	const;
		iterator		end();
		const_iterator	end()	const;
};

#endif

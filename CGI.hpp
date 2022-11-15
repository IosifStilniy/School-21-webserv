#ifndef CGI_HPP
# define CGI_HPP

# include <string>
# include <map>
# include <cstdlib>
# include <algorithm>
# include <unistd.h>
# include <csignal>
# include <ctime>

# include "Request.hpp"
# include "Polls.hpp"
# include "typedefs.hpp"

# include "utils.hpp"

# ifndef CGI_TIMEOUT
#  define CGI_TIMEOUT 5
# endif

class Response;

class CGI
{
	private:
		std::string							_path;
		std::map<std::string, std::string>	_env;

		int	in;
		int	out;

		pid_t	pid;
		int		stat_loc;

		Polls	polls;

		ByteTypes::byte_type *	buf;
		size_t					buf_size;

		std::time_t	last_modified;

		void						_runChild(int server_to_cgi[2], int cgi_to_server[2]);
		void						_setup(Request & request, Response & response);
		void						_setEnv(Request & request, std::string const & mounted_path, std::string const & loc_path);
		std::string::const_iterator	_getPathEdge(std::string const & path, std::string const & delim);
		std::string					_extractPathInfo(std::string const & requested_path, std::string const & loc_path);
		std::string					_translatePath(std::string const & mounted_path, std::string const & loc_path);
		std::string					_extractQueryString(std::string const & requested_path);
		ft::splited_string			_vectorizeEnv(void);
		void						_getHeaderFromCGI(Response & response, ByteTypes::bytes_type & chunk);
		void						_setResponseStatus(Response & response);
		bool						_msgRecieved(Response & response);
		void						_finishRecieving(Response & response);

	public:
		CGI(void);
		~CGI();

		void				handle(Request & request, Response & response);
		void				setPath(std::map<std::string, std::string> & cgi, std::string const & method);
		std::string const &	getPath(void);
};

#endif

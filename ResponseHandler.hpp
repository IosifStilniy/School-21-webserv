#ifndef RESPONSEHANDLER_HPP
# define RESPONSEHANDLER_HPP

# include <sys/socket.h>

# include "Maintainer.hpp"

# ifndef NL
#  define NL "\r\n"
# endif

class ResponseHandler
{
	private:
		typedef	Maintainer::response_queue	response_queue;
		typedef	Maintainer::Response		response_type;
		typedef	Maintainer::header_fields	header_fields;
		typedef Maintainer::bytes_type		bytes_type;

		static const std::map<int, std::string>	_statuses;

		std::string	_formHeader(header_fields & options, int status);

	public:
		ResponseHandler(void);
		~ResponseHandler();

		void	giveResponse(Maintainer::response_queue & resp_queue, int socket);
};

#endif

#ifndef RESPONSEHANDLER_HPP
# define RESPONSEHANDLER_HPP

# include <sys/socket.h>
# include <unistd.h>

# include "Maintainer.hpp"

# include "utils.hpp"

# ifndef NL
#  define NL "\r\n"
# endif

class ResponseHandler
{
	private:
		typedef	Maintainer::response_queue	response_queue;
		typedef	Response					response_type;
		typedef	Response::header_fields		header_fields;
		typedef ByteTypes::bytes_type		bytes_type;

		std::string	_formHeader(header_fields & options, int status);

	public:
		ResponseHandler(void);
		~ResponseHandler();

		void	giveResponse(response_queue & resp_queue, int socket);
};

#endif

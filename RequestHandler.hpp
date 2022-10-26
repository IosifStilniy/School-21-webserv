#ifndef REQUESTHANDLER_CPP
# define REQUESTHANDLER_CPP

# include <string>
# include <sys/socket.h>
# include <vector>
# include <map>
# include <queue>
# include <stdexcept>
# include <algorithm>

# ifndef KB
#  define KB(x) (x * 1024)
# endif

# ifndef MB
#  define MB(x) (KB(x) * 1024)
# endif

# ifndef BUFSIZE
#  define BUFSIZE KB(4)
# endif

# ifndef HTTP_EOF
#  define HTTP_EOF "\r\n\r\n"
# endif

class RequestHandler
{
	public:
		typedef	char						byte_type;
		typedef	std::vector<byte_type>		bytes_type;
		typedef bytes_type::const_iterator	bytes_iterator;

		struct Request
		{
			bytes_type		bytes;
			bool			msg_expacted;

			Request(void);

			template <typename Iterator>
			Request(Iterator begin, Iterator end, bool msg_expacted)
				: bytes(begin, end), msg_expacted(msg_expacted)
			{};
		};

		typedef std::queue<Request>				request_queue;
		typedef	std::map<int, request_queue >	socket_map;

	private:
		socket_map	_sockets;
		byte_type	_buf[BUFSIZE];
		ssize_t		_ret;
		byte_type *	_msg_end;
		byte_type *	_msg_eof;

		static std::string	_eof;

		template <typename Iterator>
		static Iterator	_getEOF(Iterator begin, Iterator end)
		{
			return (std::search(begin, end, RequestHandler::_eof.begin(), RequestHandler::_eof.end()));
		};

		bool	_handle(request_queue & req_lst);
		bool	_isHeader(std::string request_line);

	public:
		RequestHandler(void);
		~RequestHandler();

		void	collectAndHandle(int socket);
};

#endif

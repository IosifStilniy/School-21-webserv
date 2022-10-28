#ifndef REQUESTHANDLER_CPP
# define REQUESTHANDLER_CPP

# include <string>
# include <sys/socket.h>
# include <vector>
# include <map>
# include <queue>
# include <stdexcept>
# include <algorithm>

# include "utils.hpp"

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
		typedef unsigned char						byte_type;
		typedef	std::vector<byte_type>				bytes_type;
		typedef bytes_type::const_iterator			bytes_iterator;
		typedef std::map<std::string, std::string>	header_fields;

		struct Request
		{
			bytes_type		bytes;
			header_fields	options;
			size_t			content_length;
			std::string		transfer_encoding;
			bool			is_received;

			Request(void);

			template <typename Iterator>
			Request(Iterator begin, Iterator end, size_t msg_length)
				: bytes(begin, end), msg_length(msg_length), is_msg(msg_length)
			{};

			void	parseHeader(void);
			bool	isFullyReceived(void);
		};

		typedef std::queue<Request>				request_queue;
		typedef	std::map<int, request_queue>	socket_map;

	private:
		socket_map	_sockets;
		byte_type	_buf[BUFSIZE];

		static std::string	_eof;

		template <typename Iterator>
		static Iterator	_getEOF(Iterator begin, Iterator end)
		{
			return (std::search(begin, end, RequestHandler::_eof.begin(), RequestHandler::_eof.end()));
		};

		size_t		_handle(Request & request);
		byte_type *	_splitIncomingStream(Request & request, byte_type * msg_start, byte_type * msg_end);
		bool		_transferEnded(byte_type ** msg_start, size_t dstnc);
		byte_type *	_chunkedTransferHandler(Request & request, byte_type * msg_start, byte_type * msg_end);

	public:
		RequestHandler(void);
		~RequestHandler();

		void	collectAndHandle(int socket);
};

#endif

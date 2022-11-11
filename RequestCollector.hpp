#ifndef REQUESTCOLLECTOR_CPP
# define REQUESTCOLLECTOR_CPP

# include <string>
# include <sys/socket.h>
# include <vector>
# include <map>
# include <queue>
# include <stdexcept>
# include <algorithm>
# include <cstdlib>
# include <iostream>

# include "utils.hpp"

# include "typedefs.hpp"
# include "Request.hpp"

# ifndef KB
#  define KB(x) (x * 1024)
# endif

# ifndef MB
#  define MB(x) (KB(x) * 1024)
# endif

# ifndef BUFSIZE
#  define BUFSIZE KB(4)
// #  define BUFSIZE 4
# endif

# ifndef HTTP_EOF
#  define HTTP_EOF "\r\n\r\n"
# endif

class RequestCollector
{
	public:
		typedef ByteTypes::byte_type						byte_type;
		typedef	ByteTypes::bytes_type						bytes_type;
		typedef ByteTypes::chunks_type						chunks_type;
		typedef ByteTypes::bytes_iterator					bytes_iterator;
		typedef std::map<std::string, std::string>			header_values_params;
		typedef std::map<std::string, header_values_params>	header_values;
		typedef std::map<std::string, header_values>		header_fields;

		typedef std::queue<Request>				request_queue;
		typedef	std::map<int, request_queue>	socket_map;

		typedef socket_map::iterator			iterator;
		typedef socket_map::const_iterator		const_iterator;

	private:
		static const std::string	_eof;

		socket_map			_sockets;
		byte_type *			_buf;
		const std::string &	_ref_eof;

		template <typename Iterator>
		Iterator	_getEOF(Iterator begin, Iterator end)
		{
			return (std::search(begin, end, this->_ref_eof.begin(), this->_ref_eof.end()));
		};

		byte_type *	_readHeader(Request & request, byte_type * msg_start, byte_type * msg_end);
		byte_type *	_splitIncomingStream(Request & request, byte_type * msg_start, byte_type * msg_end);
		bool		_isSplitedEOF(bytes_type & chunk, byte_type * & msg_start, byte_type * msg_end);
		bool		_transferEnded(byte_type * & msg_start, size_t dstnc);
		byte_type *	_chunkedTransferHandler(Request & request, byte_type * msg_start, byte_type * msg_end);

	public:
		RequestCollector(void);
		~RequestCollector();

		request_queue &	operator[](int socket);

		void	collect(int socket);
		void	erase(const int socket);

		iterator		begin();
		const_iterator	begin()	const;
		iterator		end();
		const_iterator	end()	const;
};

#endif

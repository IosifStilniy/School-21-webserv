#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <vector>
# include <map>
# include <string>
# include <iostream>
# include <ctime>

# include "utils.hpp"
# include "typedefs.hpp"

# ifndef METHOD
#  define METHOD "method"
# endif

# ifndef CONTENT_PATH
#  define CONTENT_PATH "content-path"
# endif

# ifndef HTTP_V
#  define HTTP_V "http-version"
# endif

# ifndef HTTP_EOF
#  define HTTP_EOF "\r\n\r\n"
# endif

# ifndef NL
#  define NL "\r\n"
# endif

# ifndef REQ_TIMEOUT
#  define REQ_TIMEOUT 5
# endif

class Request
{
	public:
		enum transfering {tStd, tChunked};

		typedef ByteTypes::byte_type						byte_type;
		typedef	ByteTypes::bytes_type						bytes_type;
		typedef ByteTypes::chunks_type						chunks_type;
		typedef std::map<std::string, std::string>			header_values_params;
		typedef std::map<std::string, header_values_params>	header_values;
		typedef std::map<std::string, header_values>		header_fields;

	public:
		chunks_type		chunks;
		header_fields	options;
		size_t			content_length;
		bool			is_good;
		transfering		tr_state;
		std::string		raw_header;

		std::time_t		last_modified;

		static const std::string	nl;
		static const std::string	eof;

		Request(void);

	private:
		std::string		_tail;

	public:
		void				parseHeader(void);
		byte_type *			getChunkSize(byte_type * msg_start, byte_type * msg_end);
		bool				isFullyReceived(void);
		std::string const &	getOnlyValue(std::string const & field);
		std::string const &	getOnlyValue(header_fields::iterator field);
		void				setValues(ft::key_value_type const & key_value);
		std::string			formOptionLine(std::string const & field);
		size_t				getContentLength(void);
		bool				empty(void);
		bool				isStale(void);

		void	printOptions(void);
		void	printOptions(header_values_params const & options, int indent = 0);

		template <typename T>
		void	printOptions(std::map<std::string, T> const & options, int indent = 0)
		{
			for (typename std::map<std::string, T>::const_iterator start = options.begin(); start != options.end(); start++)
			{
				std::cout.width(indent);
				std::cout << "";
				std::cout << start->first;
				if (start->second.size())
				{
					std::cout << ": " << std::endl;
					printOptions(start->second, indent + 4);
				}
				else
					std::cout << ";" << std::endl;
			}
		};
};

#endif

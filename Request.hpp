#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <vector>
# include <queue>
# include <map>
# include <string>
# include <iostream>

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

class Request
{
	public:
		enum transfering {tStd, tChunked};

		typedef ByteTypes::byte_type						byte_type;
		typedef	ByteTypes::bytes_type						bytes_type;
		typedef ByteTypes::chunks_type						chunks_type;
		typedef ByteTypes::bytes_iterator					bytes_iterator;
		typedef std::map<std::string, std::string>			header_values_params;
		typedef std::map<std::string, header_values_params>	header_values;
		typedef std::map<std::string, header_values>		header_fields;

	public:
		chunks_type		chunks;
		header_fields	options;
		size_t			content_length;
		bool			is_good;
		transfering		tr_state;

		Request(void);

	public:
		void				parseHeader(void);
		bool				isFullyReceived(void);
		std::string const &	getOnlyValue(std::string const & field);
		std::string const &	getOnlyValue(header_fields::iterator field);
		void				setValues(ft::key_value_type const & key_value);
		bool				empty(void);

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

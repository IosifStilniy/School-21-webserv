#ifndef UTILS_HPP
# define UTILS_HPP

# include <vector>
# include <queue>
# include <algorithm>
# include <stdexcept>
# include <string>
# include <cctype>
# include <cstring>
# include <cmath>
# include <fstream>

# ifndef SPACES
#  define SPACES " \t\n\v\f\r"
# endif

# ifndef QUOTE_CHARS
#  define QUOTE_CHARS "\"\"''()"
# endif

namespace ft
{
	namespace
	{
		typedef struct s_quots
		{
			std::string::const_iterator	begin;
			std::string::const_iterator	end;
		}	t_quots;
	}

	typedef	std::vector<std::string>				splited_string;
	typedef	std::pair<std::string, std::string>		key_value_type;

	std::string		trim(std::string const & str);
	splited_string	split(std::string const & str, std::string delimiters = SPACES, bool ignore_quotes = false);
	std::string		toLower(std::string const & str);
	key_value_type	splitHeader(std::string const & str, std::string const & separators);
	bool			isDirectory(std::string const & filename);
	void			readConfFile(std::ifstream & conf_file, std::string & line);
	size_t			removePrefixB(std::string const & size);

	template <typename Number>
	std::string	num_to_string(Number num)
	{
		std::string	str;
		bool		sign = (num < 0);

		str.reserve(50);
		while (num)
		{
			str.insert(str.begin(), (num % 10) + '0');
			num /= 10;
		}

		if (sign)
			str.insert(str.begin(), '-');

		return (str);
	};

	template <typename FileStream>
	void	openFile(FileStream & file, std::string const & filename, std::ios_base::openmode openmode = std::ios_base::out)
	{
		if (ft::isDirectory(filename))
			throw std::runtime_error(filename + ": " + strerror(errno));

		if (file.is_open())
			file.close();

		file.open(filename, openmode);

		if (!file.good())
		{
			if (file.is_open())
				file.close();
				
			throw std::runtime_error(filename + ": " + strerror(errno));
		}
	}
}

#endif

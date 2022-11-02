#ifndef UTILS_HPP
# define UTILS_HPP

# include <vector>
# include <queue>
# include <algorithm>
# include <stdexcept>
# include <string>
# include <cctype>
# include <cstring>
# include <fstream>

# ifndef SPACES
#  define SPACES " \t\n\v\f\r"
# endif

# ifndef QUOTE_CHARS
#  define QUOTE_CHARS "\"\"''()"
# endif

namespace _ft
{
	typedef struct s_quots
	{
		std::string::const_iterator	begin;
		std::string::const_iterator	end;
	}	t_quots;

	std::queue<t_quots>	fillQuots(std::string const & str, std::string quots_string = QUOTE_CHARS);
}

namespace ft
{
	typedef	std::vector<std::string>	splited_string;

	std::string		trim(std::string const & str);
	splited_string	split(std::string const & str, std::string delimiters = SPACES, bool ignore_quotes = false);
	std::string		toLower(std::string const & str);
	splited_string	splitHeader(std::string const & str, std::string const & separators);
	bool			isDirectory(std::string const & filename);
	void			readConfFile(std::ifstream & conf_file, std::string & line);

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
	void	openFile(FileStream & file, std::string const & filename)
	{
		if (ft::isDirectory(filename))
			throw std::runtime_error(filename + ": " + strerror(errno));

		file.open(filename);

		if (!file.good())
		{
			if (file.is_open())
				file.close();
				
			throw std::runtime_error(filename + ": " + strerror(errno));
		}
	}
}

#endif
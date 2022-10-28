#ifndef UTILS_HPP
# define UTILS_HPP

# include <vector>
# include <queue>
# include <algorithm>
# include <stdexcept>
# include <string>
# include <cctype>

# ifndef SPACES
#  define SPACES " \t\n\v\f\r"
# endif

# ifndef QUOTE_CHARS
#  define QUOTE_CHARS "\"'"
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
}

#endif
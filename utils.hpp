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
	void	openFile(FileStream & file, std::string const & filename, std::ios_base::openmode openmode = std::ios_base::in)
	{
		if (isDirectory(filename))
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

	template <typename SrcContainer, typename SearchContainer, typename ReplContainer>
	SrcContainer	replaceBytes(SrcContainer const & src, SearchContainer const & what_replace, ReplContainer const & replace_with)
	{
		typedef typename std::enable_if<std::is_same<typename SrcContainer::value_type, typename SearchContainer::value_type>::value, typename SrcContainer::value_type>::type		_check1;
		typedef typename std::enable_if<std::is_same<typename SrcContainer::value_type, typename ReplContainer::value_type>::value, typename SrcContainer::value_type>::type		_check2;

		if (what_replace.empty())
			return (src);

		SrcContainer							replaced_sequence;
		typename SrcContainer::const_iterator	left = src.begin();
		typename SrcContainer::const_iterator	right = std::search(left, src.end(), what_replace.begin(), what_replace.end());

		while (left != src.end())
		{
			replaced_sequence.insert(replaced_sequence.end(), left, right);

			if (right != src.end())
				replaced_sequence.insert(replaced_sequence.end(), replace_with.begin(), replace_with.end());
			else
				return (replaced_sequence);

			left = right + what_replace.size();
			right = std::search(left, src.end(), what_replace.begin(), what_replace.end());
		}
		
		return (replaced_sequence);
	}
}

#endif

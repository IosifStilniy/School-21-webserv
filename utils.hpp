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
# include <iostream>

# include "typedefs.hpp"

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
	std::string		toUpper(std::string const & str);
	key_value_type	splitHeader(std::string const & str, std::string const & separators);
	bool			isDirectory(std::string const & filename);
	bool			exist(std::string const & filename);
	void			readConfFile(std::ifstream & conf_file, std::string & line);
	size_t			removePrefixB(std::string const & size);
	std::string		returnLine(std::string const & line);

	template <typename Number>
	std::string	num_to_string(Number num)
	{
		if (!num)
			return (std::string("0"));

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
		if (file.is_open())
			file.close();
		
		file.open(filename, openmode);
		if (!file.good())
		{
			if (file.is_open())
				file.close();

			throw std::runtime_error(filename + ": " + strerror(errno));
		}
		
		if ((openmode & std::ios_base::in) != std::ios_base::in)
			return ;

		if (isDirectory(filename))
			throw std::runtime_error(filename + ": " + strerror(errno));
	}

	template <typename SrcContainer, typename SearchContainer, typename ReplContainer>
	SrcContainer	replaceBytes(SrcContainer const & src, SearchContainer const & what_replace, ReplContainer const & replace_with)
	{
		typedef typename std::enable_if<std::is_same<typename SrcContainer::value_type, typename SearchContainer::value_type>::value, SrcContainer>::type	_SrcContainer;
		typedef typename std::enable_if<std::is_same<typename SrcContainer::value_type, typename ReplContainer::value_type>::value, _SrcContainer>::type	__SrcContainer;

		if (what_replace.empty())
			return (src);

		__SrcContainer							replaced_sequence;
		typename __SrcContainer::const_iterator	left = src.begin();
		typename __SrcContainer::const_iterator	right = std::search(left, src.end(), what_replace.begin(), what_replace.end());

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

	template <typename SrcContainer, typename SearchContainer, typename ReplContainer>
	SrcContainer	replaceBytesOnce(SrcContainer const & src, SearchContainer const & what_replace, ReplContainer const & replace_with)
	{
		typedef typename std::enable_if<std::is_same<typename SrcContainer::value_type, typename SearchContainer::value_type>::value, SrcContainer>::type	_SrcContainer;
		typedef typename std::enable_if<std::is_same<typename SrcContainer::value_type, typename ReplContainer::value_type>::value, _SrcContainer>::type	__SrcContainer;

		if (what_replace.empty())
			return (src);

		__SrcContainer							replaced_sequence;
		typename __SrcContainer::const_iterator	right = std::search(src.begin(), src.end(), what_replace.begin(), what_replace.end());

		replaced_sequence.insert(replaced_sequence.end(), src.begin(), right);

		if (right == src.end())
			return (replaced_sequence);

		replaced_sequence.insert(replaced_sequence.end(), replace_with.begin(), replace_with.end());
		replaced_sequence.insert(replaced_sequence.end(), right + what_replace.size(), src.end());
		
		return (replaced_sequence);
	}

	template <typename Container, class Func>
	void	containerazeConfFile(Container & cont, std::string const & filename, Func func = &returnLine)
	{
		std::ifstream	file;

		openFile(file, filename);

		std::string	line;

		while (file.good())
		{
			readConfFile(file, line);

			if (line.empty())
				continue ;

			cont.insert(cont.end(), func(line));
		}

		if (file.is_open())
			file.close();

		if (!file.eof())
			throw std::runtime_error(filename + ": " + strerror(errno));
	}

	template <typename Container, class Func>
	Container	containerazeConfFile(std::string const & filename, Func func = &returnLine)
	{
		Container 	cont;
		containerazeConfFile(cont, filename, func);
		return (cont);
	}

	template <typename Container>
	char * const *	containerToArray(Container const & cont)
	{
		const char **	arr = reinterpret_cast<const char **>(malloc(sizeof(*arr) * (cont.size() + 1)));
		const char **	crsr = arr;

		for (typename Container::const_iterator string = cont.begin(); string != cont.end(); string++, crsr++)
			*crsr = string->c_str();
		crsr = NULL;

		return (const_cast<char * const *>(arr));
	}
}

#endif

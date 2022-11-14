#include "utils.hpp"

namespace ft
{
	namespace
	{
		static std::queue<t_quots>	fillQuots(std::string const & str, std::string quots_string = QUOTE_CHARS)
		{
			t_quots						quots;
			std::queue<t_quots>			quots_lst;
			std::string::const_iterator	quote_type;
			size_t						op_count = 0;

			quots.begin = std::find_first_of(str.begin(), str.end(), quots_string.begin(), quots_string.end());
			quote_type = std::find(quots_string.begin(), quots_string.end(), *quots.begin);

			while (quots.begin != str.end())
			{
				quots.end = quots.begin;

				while (quots.end != str.end()
					&& (*quots.end != *(quote_type + 1) || op_count || quots.end == quots.begin))
				{
					if (*quots.end == *quote_type && *quote_type != *(quote_type + 1))
						op_count++;
					
					quots.end = std::find_first_of(quots.end + 1, str.end(), quots_string.begin(), quots_string.end());

					if (!op_count || *quots.end != *(quote_type + 1))
						continue ;
					
					op_count--;
				}

				if (quots.end == str.end())
					return (quots_lst);
				
				quots_lst.push(quots);

				quots.begin = std::find_first_of(quots.end + 1, str.end(), quots_string.begin(), quots_string.end());
				quote_type = std::find(quots_string.begin(), quots_string.end(), *quots.begin);
			}

			return (quots_lst);
		}

		static bool	isQuoted(std::string::const_iterator symb, std::queue<t_quots> quots)
		{
			if (!quots.size())
				return (false);

			while (quots.size() && quots.front().end < symb)
				quots.pop();
			
			if (quots.front().begin < symb && symb < quots.front().end)
				return (true);

			return (false);
		}
	};

	std::string	trim(std::string const & str)
	{
		std::string::const_iterator			begin = str.begin();
		std::string::const_reverse_iterator	rbegin = str.rbegin();

		for (; begin != str.end(); begin++)
			if (!isspace(*begin))
				break ;
		
		if (begin == str.end())
			return ("");

		for (; rbegin != str.rend(); rbegin++)
			if (!isspace(*rbegin))
				break ;
		
		return (std::string(begin, rbegin.base()));
	}

	key_value_type	splitHeader(std::string const & str, std::string const & separators)
	{
		key_value_type				key_value;
		std::string::const_iterator	delim = std::find_first_of(str.begin(), str.end(), separators.begin(), separators.end());

		key_value.first = ft::trim(std::string(str.begin(), delim));

		// while (delim != str.end() && separators.find(*delim) != std::string::npos)
		// 	delim++;

		if (delim == str.end())
			return (key_value);
		
		key_value.second = ft::trim(std::string(delim + 1, str.end()));

		return (key_value);
	}

	splited_string	split(std::string const & str, std::string const & delimiters, bool ignore_quotes)
	{
		std::queue<t_quots>				quots_lst;
		splited_string					splited;
		std::string::const_iterator		begin = str.begin();
		std::string::const_iterator		end = str.begin();

		if (!ignore_quotes)
			quots_lst = fillQuots(str);

		while (end != str.end())
		{
			if (!isQuoted(end, quots_lst) && delimiters.find(*begin) != std::string::npos)
			{
				++begin;
				++end;
			}
			else if (delimiters.find(*end) == std::string::npos || isQuoted(end, quots_lst))
				++end;
			else
			{
				splited.push_back(std::string(begin, end));
				begin = end;
			}
		}

		if (begin != end)
			splited.push_back(std::string(begin, end));
		
		if (splited.empty())
			splited.push_back(std::string());

		return (splited);
	}

	splited_string	splitByComplexDelimeter(std::string const & str, std::string const &  delimiter, bool ignore_quotes)
	{
		std::queue<t_quots>					quots_lst;
		splited_string						splited;
		std::string::const_iterator			begin = str.begin();
		std::string::const_iterator			end = str.begin();

		if (!ignore_quotes)
			quots_lst = fillQuots(str);

		while (end < str.end())
		{
			if (!isQuoted(end, quots_lst) && std::search(begin, str.end(), delimiter.begin(), delimiter.end()) == begin)
			{
				begin += delimiter.size();
				end += delimiter.size();
			}
			else if (std::search(end, str.end(), delimiter.begin(), delimiter.end()) != end || isQuoted(end, quots_lst))
				end++;
			else
			{
				splited.push_back(std::string(begin, end));
				begin = end;
			}
		}

		if (begin != end)
			splited.push_back(std::string(begin, end));
		
		if (splited.empty())
			splited.push_back(std::string());

		return (splited);
	}

	std::string		toLower(std::string const & str)
	{
		std::string	lower = str;

		for (std::string::iterator start = lower.begin(), end = lower.end(); start != end; start++)
			*start = tolower(*start);
		
		return (lower);
	}

	std::string		toUpper(std::string const & str)
	{
		std::string	lower = str;

		for (std::string::iterator start = lower.begin(), end = lower.end(); start != end; start++)
			*start = toupper(*start);
		
		return (lower);
	}

	bool	exist(std::string const & filename)
	{
		std::ifstream	ifile(filename);

		ifile.close();

		if (!ifile.good())
			return (false);

		return (true);
	}

	bool	isDirectory(std::string const & filename)
	{
		if (!exist(filename))
			return (false);

		std::ofstream	file(filename, std::ios_base::app);

		file.close();

		if (file.good())
			return (false);

		if (errno == 21)
			return (true);
		
		return (false);
	}

	void	readConfFile(std::ifstream & conf_file, std::string & line)
	{
		if (conf_file.eof())
		{
			line.clear();
			return ;
		}

		std::getline(conf_file, line);

		line = ft::trim(line.substr(0, line.find('#')));
	}

	size_t	removePrefixB(std::string const & size)
	{
		char *	word;
		size_t	num = strtoul(size.c_str(), &word, 10);
		size_t	exp = std::string("kmgt").find(tolower(*word));

		if (exp == std::string::npos)
			return (num);
		return (num * std::pow(2, 10 * (exp + 1)));
	}

	std::string	returnLine(std::string const & line)
	{
		return (line);
	}

	std::string	quoteString(std::string const & string, std::string const & quote)
	{
		return (quote + string + quote);
	}
};

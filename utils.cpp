#include "utils.hpp"

namespace _ft
{
	std::queue<t_quots>	fillQuots(std::string const & str, std::string quots_string)
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

	bool	isQuoted(std::string::const_iterator symb, std::queue<t_quots> quots)
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

namespace ft
{
	std::string	trim(std::string const & str)
	{
		std::string::const_iterator			begin = str.begin();
		std::string::const_reverse_iterator	rbegin = str.rbegin();

		for (; begin != str.end(); begin++)
			if (!isspace(*begin))
				break ;
		

		for (; rbegin != str.rend(); rbegin++)
			if (!isspace(*rbegin))
				break ;
		
		return (std::string(begin, rbegin.base()));
	}

	splited_string	splitHeader(std::string const & str, std::string const & separators)
	{
		splited_string				splited;

		if (str.empty())
			return (splited);

		std::string::const_iterator	delim = std::find_first_of(str.begin(), str.end(), separators.begin(), separators.end());

		splited.push_back(std::string(str.begin(), delim));

		if (delim == str.end())
			return (splited);

		splited.push_back(std::string(delim + 1, str.end()));

		return (splited);
	}

	splited_string	split(std::string const & str, std::string delimiters, bool ignore_quotes)
	{
		std::queue<_ft::t_quots>		quots_lst;
		splited_string					splited;
		std::string::const_iterator		begin = str.begin();
		std::string::const_iterator		end = str.begin();

		if (!ignore_quotes)
			quots_lst = _ft::fillQuots(str);

		while (end != str.end())
		{
			if (!_ft::isQuoted(end, quots_lst) && delimiters.find(*begin) != std::string::npos)
			{
				++begin;
				++end;
			}
			else if (delimiters.find(*end) == std::string::npos || _ft::isQuoted(end, quots_lst))
				++end;
			else
			{
				splited.push_back(std::string(begin, end));
				begin = end;
			}
		}

		if (begin != end)
			splited.push_back(std::string(begin, end));

		return (splited);
	}

	std::string		toLower(std::string const & str)
	{
		std::string	lower = str;

		for (std::string::iterator start = lower.begin(), end = lower.end(); start != end; start++)
			*start = tolower(*start);
		
		return (lower);
	}

	bool	isDirectory(std::string const & filename)
	{
		std::ofstream	file(filename);

		if (file.is_open())
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
};

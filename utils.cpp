#include "utils.hpp"

namespace _ft
{
	std::queue<t_quots>	fillQuots(std::string const & str, std::string quots_string)
	{
		t_quots						quots;
		std::queue<t_quots>			quots_lst;

		quots.begin = std::find_first_of(str.begin(), str.end(), quots_string.begin(), quots_string.end());

		while (quots.begin != str.end())
		{
			quots.end = std::find_first_of(quots.begin + 1, str.end(), quots_string.begin(), quots_string.end());

			if (quots.end == str.end())
				throw std::out_of_range("quotes not closed");
			
			quots_lst.push(quots);

			quots.begin = std::find_first_of(quots.end + 1, str.end(), quots_string.begin(), quots_string.end());
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
		std::string::const_iterator	begin = str.begin();
		std::string::const_iterator	end = str.end();

		for (; begin != end; begin++)
			if (!isspace(*begin))
				break ;
		

		for (; end != begin; end--)
			if (!isspace(*(end - 1)))
				break ;
		
		return (std::string(begin, end));
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
};
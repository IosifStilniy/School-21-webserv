#include <string>
#include <cctype>

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
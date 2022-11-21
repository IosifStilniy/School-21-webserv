#include <iostream>
#include <ctime>
#include "../srcs/utils/utils.hpp"

void	send_text(std::string const & txt)
{
	std::cout << "Content-Type: text/plain" << std::endl;
	std::cout << "Content-Length: " + ft::num_to_string(txt.size()) << "\r\n\r\n";
	std::cout << txt << std::endl;
}

int main()
{
	char *	cookie_env = getenv("HTTP_COOKIE");
	std::string	cookie;

	if (cookie_env)
		cookie = cookie_env;
	
	if (!cookie.empty())
	{
		send_text("You already have cookies. There it is: " + cookie);
		return (0);
	}

	cookie = "T" + ft::num_to_string(time(NULL));
	std::cout << "Set-Cookie: " + cookie << std::endl;
	send_text("Here your delicious cookies, ma boyyyy: " + cookie);
	return (0);
}
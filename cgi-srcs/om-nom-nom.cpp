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
	
	if (cookie.empty())
	{
		send_text("Your cookies was already eaten...");
		return (0);
	}

	std::string	eaten = cookie + "; Max-Age=0";
	std::cout << "Set-Cookie: " + eaten << std::endl;
	send_text("Om-nom-nom... mmm, " + cookie + " was so sweet, ma boy!");
	return (0);
}
#include <iostream>
#include <string>
#include <unistd.h>
#include "../srcs/utils/utils.hpp"

int main()
{
	std::string	txt = "<h1>simple cgi test</h1>";

	std::cout << "Content-Length: " << ft::num_to_string(txt.size()) << std::endl;
	std::cout << "Content-Type: text/html\r\n\r\n";
	std::cout << txt << std::endl;
	return (0);
}
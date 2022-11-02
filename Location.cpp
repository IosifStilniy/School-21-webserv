#include "Location.hpp"

Location::Location(void)
	: buf_size(BUFSIZE)
{
	this->indexes.push_back("index.html");
	this->methods.push_back("get");
	this->methods.push_back("post");
	this->methods.push_back("delete");
}

Location::~Location()
{
}
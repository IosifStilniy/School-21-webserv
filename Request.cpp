#include "Request.hpp"

Request::Request(void)
	: chunks(), options(), content_length(0), transfer_encoding(), is_ready(false)
{
	chunks.push(bytes_type());
}

void	Request::setValues(ft::key_value_type const & field_values)
{
	ft::splited_string	splited_values = ft::split(field_values.second, ",");

	if (splited_values[0].empty())
		return ;

	ft::splited_string	params;
	ft::key_value_type	key_value;

	this->options[field_values.first];
	for (ft::splited_string::const_iterator start = splited_values.begin(); start != splited_values.end(); start++)
	{
		params = ft::split(*start, ";");
		params[0] = ft::trim(params[0]);

		this->options[field_values.first][params[0]];
		for (ft::splited_string::const_iterator start = params.begin() + 1; start != params.end(); start++)
		{
			key_value = ft::splitHeader(*start, "=");
			this->options[field_values.first][params[0]][key_value.first] = key_value.second;
		}
	}
}

std::string const &	Request::getOnlyValue(header_fields::iterator field)
{
	if (field->second.empty())
		field->second[""];
	return (field->second.rbegin()->first);
}

std::string const &	Request::getOnlyValue(std::string const & field)
{
	if (this->options[field].empty())
		this->options[field][""];
	return (this->options[field].rbegin()->first);
}

void	Request::parseHeader(void)
{
	if (!this->options.empty())
		return ;

	ft::splited_string	splited = ft::split(std::string(this->chunks.front().begin(), this->chunks.front().end()), "\n");
	ft::splited_string	splited_line = ft::split(splited[0]);

	this->setValues(std::make_pair(METHOD, ft::toLower(splited_line[0])));
	this->setValues(std::make_pair(CONTENT_PATH, splited_line[1]));
	this->setValues(std::make_pair(HTTP_V, splited_line[2]));

	for (ft::splited_string::iterator start = splited.begin() + 1; start != splited.end(); start++)
		this->setValues(ft::splitHeader(*start, ":"));

	header_fields::iterator	length = this->options.find("Content-Length");

	if (length != this->options.end())
		this->content_length = strtoul(this->getOnlyValue(length).c_str(), NULL, 10);

	length = this->options.find("Transfer-Encoding");

	if (length != this->options.end())
		this->transfer_encoding = length->second;
	
	if (!this->content_length || this->transfer_encoding.find("chunked") != this->transfer_encoding.end())
		this->is_ready = true;
}

bool	Request::isFullyReceived(void)
{
	if (this->options.empty())
		return (false);

	if ((!this->content_length
		&& (this->transfer_encoding.find("chunked") != this->transfer_encoding.end()))
		|| (this->content_length && this->content_length == this->chunks.front().size())
	)
	{
		this->is_ready = true;
		return (true);
	}

	return (false);
}

void	Request::printOptions(header_values_params const & options, int indent)
{
	for (header_values_params::const_iterator start = options.begin(); start != options.end(); start++)
	{
		std::cout.width(indent);
		std::cout << "";
		std::cout << start->first << " = " << start->second << std::endl;
	}
}

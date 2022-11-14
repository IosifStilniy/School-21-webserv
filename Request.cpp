#include "Request.hpp"

const std::string Request::eof(HTTP_EOF);
const std::string Request::nl(NL);

Request::Request(void)
	: chunks(), options(), content_length(0), is_good(true), tr_state(tStd)
{
	chunks.push_back(bytes_type());
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

	if (splited_line.size() != 3)
		throw std::runtime_error("bad request-line: " + splited[0]);

	this->setValues(std::make_pair(METHOD, ft::toLower(splited_line[0])));
	this->setValues(std::make_pair(CONTENT_PATH, splited_line[1]));
	this->setValues(std::make_pair(HTTP_V, splited_line[2]));

	for (ft::splited_string::iterator start = splited.begin() + 1; start != splited.end(); start++)
		this->setValues(ft::splitHeader(*start, ":"));

	this->content_length = strtoul(this->getOnlyValue("Content-Length").c_str(), NULL, 10);

	if (this->options["Transfer-Encoding"].find("chunked") != this->options["Transfer-Encoding"].end())
		this->tr_state = tChunked;
}

bool	Request::isFullyReceived(void)
{
	return (!this->options.empty() && !this->content_length && this->tr_state == tStd);
}

void	Request::printOptions(void)
{
	this->printOptions(this->options);
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

bool	Request::empty(void)
{
	return ((this->chunks.empty() || this->chunks.front().empty()) && !this->content_length && this->tr_state == tStd);
}

Request::byte_type *	Request::getChunkSize(byte_type * msg_start, byte_type * msg_end)
{
	byte_type *	spliter = std::search(msg_start, msg_end, Request::nl.begin(), Request::nl.end());

	if (spliter == msg_start)
		spliter = std::search(msg_start + Request::nl.size(), msg_end, Request::nl.begin(), Request::nl.end());

	byte_type *	edge = spliter;

	spliter += static_cast<size_t>(msg_end - spliter) > Request::nl.size() ? Request::nl.size() : msg_end - spliter;

	this->_tail.append(msg_start, spliter);
	msg_start = spliter;

	if (this->_tail.substr(0, Request::eof.size()) == Request::eof)
	{
		this->content_length = 0;
		this->tr_state = Request::tStd;
		this->_tail.clear();
		return (msg_start);
	}

	if (edge == msg_end)
		return (msg_start);

	this->content_length = strtoul(this->_tail.c_str(), NULL, 16);
	this->_tail.clear();

	if (!this->content_length)
		return (this->getChunkSize(edge, msg_end));

	return (msg_start);
}

std::string	Request::formOptionLine(std::string const & field)
{
	std::string	line;

	for (header_values::iterator value = this->options[field].begin(); value != this->options[field].end(); value++)
	{
		line.append(value->first);

		for (header_values_params::iterator param = value->second.begin(); param != value->second.end(); param++)
		{
			line.append(";" + param->first);
			if (!param->second.empty())
				line.append("=" + param->second);
		}

		line.append(",");
	}

	if (line.back() == ',')
		line.pop_back();
	
	return (line);
}

size_t	Request::getContentLength(void)
{
	size_t	size = 0;

	for (Request::chunks_type::const_iterator chunk = this->chunks.begin(); chunk != this->chunks.end(); chunk++)
		size += chunk->size();
	
	return (size);
}

#ifndef EXCEPTIONS_HPP
# define EXCEPTIONS_HPP

# include <stdexcept>
# include <string>

# include "Response.hpp"

class BadRequestException : public std::exception
{
	protected:
		Response const &	_ref;
		std::string			_msg;

	public:
		BadRequestException(Response const & ref, std::string const & txt)
			: _ref(ref), _msg("bad request: " + txt)
		{};

		~BadRequestException()	throw()
		{};

		const char *	what(void)	const	throw()
		{
			return this->_msg.c_str();
		};
};

class ServerSettingsException : public BadRequestException
{
	public:
		ServerSettingsException(Response const & ref, std::string const & txt)
			: BadRequestException(ref, "settings: ")
		{
			this->_msg.append(txt);
		};
};

class LocationException : public BadRequestException
{
	public:
		LocationException(Response const & ref, std::string const & txt)
			: BadRequestException(ref, "location: " + ref.path_location.first + ": ")
		{
			this->_msg.append(txt);
		};
};

class PathException : public LocationException
{
	public:
		PathException(Response const & ref, std::string const & txt)
			: LocationException(ref, "mounted path: " + ref.mounted_path + ": ")
		{
			this->_msg.append(txt);
		}
};

class MethodException : public LocationException
{
	public:
		MethodException(Response const & ref, std::string const & method, std::string const & txt)
			: LocationException(ref, "method: " + method + ": ")
		{
			this->_msg.append(txt);
		}
};

class SizeLimitException : public LocationException
{
	public:
		SizeLimitException(Response const & ref, std::string const & request_size)
			: LocationException(ref, "size limit excessed: ")
		{
			this->_msg.append(request_size + "/" + ft::num_to_string(this->_ref.path_location.second->size_limit) + " B");
		}
};

#endif

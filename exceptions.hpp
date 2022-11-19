#ifndef EXCEPTIONS_HPP
# define EXCEPTIONS_HPP

# include <stdexcept>
# include <string>

# include "Response.hpp"

class IncompleteResponseException : public std::exception
{
	protected:
		std::string	_msg;
	public:
		IncompleteResponseException(std::string const & txt)
			: _msg(txt)
		{};

		~IncompleteResponseException() throw()
		{};

		const char *	what(void)	const	throw()
		{
			return this->_msg.c_str();
		};
};

class BadResponseException : public std::exception
{
	protected:
		Response &	_ref;
		std::string	_msg;

	public:
		BadResponseException(int status, Response & ref, std::string const & txt)
			: _ref(ref), _msg("bad request: " + txt)
		{
			this->_ref.badResponse(status);
		};

		~BadResponseException()	throw()
		{};

		const char *	what(void)	const	throw()
		{
			return this->_msg.c_str();
		};
};

class CGIException : public BadResponseException
{
	public:
		CGIException(Response & ref, std::string const & txt)
			: BadResponseException(500, ref, "cgi: " + txt)
		{};
};

class BadRequestException : public BadResponseException
{
	public:
		BadRequestException(int status, Response & ref, std::string const & txt)
			: BadResponseException(status, ref, "bad request: " + txt)
		{};
};

class ServerSettingsException : public BadRequestException
{
	public:
		ServerSettingsException(int status, Response & ref, std::string const & txt)
			: BadRequestException(status, ref, "settings: " + txt)
		{};
};

class LocationException : public BadRequestException
{
	public:
		LocationException(int status, Response & ref, std::string const & txt)
			: BadRequestException(status, ref, "location: " + ref.path_location->first + ": " + txt)
		{};
};

class PathException : public LocationException
{
	public:
		PathException(int status, Response & ref, std::string const & txt, std::string const & is_dir_err = std::string())
			: LocationException(status, ref, "mounted path: " + ref.mounted_path + ": " + txt)
		{
			if (!is_dir_err.empty())
				this->_ref.badResponse(status, is_dir_err);
		};
};

class MethodException : public LocationException
{
	public:
		MethodException(int status, Response & ref, std::string const & method, std::string const & txt)
			: LocationException(status, ref, "method: " + method + ": " + txt)
		{};
};

class SizeLimitException : public LocationException
{
	public:
		SizeLimitException(Response & ref, std::string const & request_size)
			: LocationException(413, ref, "size limit excessed: ")
		{
			this->_msg.append(request_size + "/" + ft::num_to_string(this->_ref.path_location->second.size_limit) + " B");
		}
};

#endif

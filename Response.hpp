#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <vector>
# include <queue>
# include <map>
# include <string>
# include <fstream>

# include "typedefs.hpp"
# include "RequestCollector.hpp"

class Response
{
	public:
		typedef ByteTypes::byte_type						byte_type;
		typedef	ByteTypes::bytes_type						bytes_type;
		typedef ByteTypes::chunks_type						chunks_type;
		typedef ByteTypes::bytes_iterator					bytes_iterator;
		typedef std::map<std::string, std::string>			header_fields;
		typedef	std::pair<std::string, Location *>			path_location_type;

		chunks_type		chunks;
		header_fields	options;
		int				status;
		bool			inited;

		ServerSettings *	settings;
		path_location_type	path_location;
		std::string			mounted_path;

		std::ifstream	in;
		std::ofstream	out;
		byte_type		buf[BUFSIZE];
		byte_type *		spliter;
		byte_type *		end;

		Response(void);
		Response(const Response & src);

	private:
		void	_getSettings(Request::header_values const & host, std::vector<ServerSettings> & settings);
		void	_getLocation(Location::locations_type & locations, std::string const & path);
		void	_checkMountedPath(void);
		void	_listIndexes(void);

	public:
		void				init(Request & request, std::vector<ServerSettings> & settings_collection);
		void				readFile(void);
		void				readFile(std::string const & filepath);
		void				badResponse(int status, std::string error_page = "");
		std::string const &	chooseErrorPageSource(void);
		std::string const &	chooseErrorPageSource(int status);
};

#endif

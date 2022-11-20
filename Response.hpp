#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <vector>
# include <map>
# include <string>
# include <list>
# include <unistd.h>
# include <fcntl.h>
# include <sys/stat.h>

# include "utils.hpp"

# include "typedefs.hpp"
# include "RequestCollector.hpp"
# include "Polls.hpp"
# include "CGI.hpp"

# ifndef MOD
#  define MOD (S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP | S_IROTH | S_IXUSR | S_IXGRP)
# endif

class Response
{
	public:
		enum connection {cStd, cKeep_alive, cClose};
		enum transfering {tStd, tChunked};

		typedef ByteTypes::byte_type						byte_type;
		typedef	ByteTypes::bytes_type						bytes_type;
		typedef ByteTypes::chunks_type						chunks_type;
		typedef std::map<std::string, std::string>			header_fields;
		typedef	std::pair<const std::string, Location> *	path_location_type;

		chunks_type		chunks;
		header_fields	options;
		int				status;
		bool			inited;

		ssize_t			content_length;

		connection		con_status;
		transfering		trans_mode;

		ServerSettings *	settings;
		path_location_type	path_location;
		std::string			mounted_path;

		Polls	polls;
		int &	in;
		int &	out;

		size_t		buf_size;
		byte_type *	buf;

		CGI			cgi;

		static std::vector<std::string>		supported_protocols;
		static std::vector<std::string>		implemented_methods;
		static std::map<int, std::string>	statuses;

		Response(void);
		Response(const Response & src);
		~Response();

	private:
		void				_getSettings(Request & request, std::vector<ServerSettings> & settings);
		void				_getLocation(Location::locations_type & locations, Request & request);
		void				_getEndPointLocation(Location::locations_type & locations, std::string const & method);
		path_location_type	_getMidPointLocation(Location::locations_type & locations, std::string const & path);
		void				_listIndexes(void);
		void				_checkSettings(Request & request);
		void				_checkLocation(Request & request);

		static std::string const	_generated_error_page;


	public:
		void				init(Request & request, std::vector<ServerSettings> & settings_collection);
		ssize_t				readFile(void);
		ssize_t				readFile(std::string const & filepath);
		void				writeFile(Request & request);
		void				badResponse(int status, std::string error_page = "");
		size_t				getContentLength(void);
		void				redirect(void);
		std::string const &	chooseErrorPageSource(void);
		std::string const &	chooseErrorPageSource(int status);
		void				checkGetPath(void);
		void				checkGetPath(std::string & path);
		void				checkPutPath(void);
		bool				empty(bool release_req = false);
};

#endif

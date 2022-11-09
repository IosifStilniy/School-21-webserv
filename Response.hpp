#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <vector>
# include <queue>
# include <map>
# include <string>
# include <list>
# include <unistd.h>
# include <fcntl.h>
# include <sys/stat.h>

# include "typedefs.hpp"
# include "RequestCollector.hpp"
# include "Polls.hpp"

# ifndef MOD
#  define MOD (S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP | S_IROTH | S_IXUSR | S_IXGRP)
# endif

class Response
{
	public:
		enum connection {cStd, cKeep_alive, cClose};
		enum transfering {tStd, tChunked};

		typedef ByteTypes::byte_type				byte_type;
		typedef	ByteTypes::bytes_type				bytes_type;
		typedef std::list<bytes_type>				chunks_type;
		typedef ByteTypes::bytes_iterator			bytes_iterator;
		typedef std::map<std::string, std::string>	header_fields;
		typedef	std::pair<std::string, Location *>	path_location_type;

		chunks_type		chunks;
		header_fields	options;
		int				status;
		bool			inited;

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
		byte_type *	spliter;
		byte_type *	end;

		static std::vector<std::string>		supported_protocols;
		static std::vector<std::string>		implemented_methods;
		static std::map<int, std::string>	statuses;

		Response(void);
		Response(const Response & src);
		~Response();

	private:
		void	_getSettings(Request & request, std::vector<ServerSettings> & settings);
		void	_getLocation(Location::locations_type & locations, Request & request);
		void	_checkGetPath(void);
		void	_checkPostPath(void);
		void	_listIndexes(void);
		void	_checkSettings(Request & request);
		void	_checkLocation(Request & request);


	public:
		void				init(Request & request, std::vector<ServerSettings> & settings_collection);
		void				readFile(void);
		void				readFile(std::string const & filepath);
		void				writeFile(Request::chunks_type & chunks);
		void				badResponse(int status, std::string error_page = "");
		void				redirect(void);
		std::string const &	chooseErrorPageSource(void);
		std::string const &	chooseErrorPageSource(int status);
};

#endif

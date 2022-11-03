#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <vector>
# include <string>

# ifndef KB
#  define KB(x) (x * 1024)
# endif

# ifndef MB
#  define MB(x) (KB(x) * 1024)
# endif

# ifndef BUFSIZE
#  define BUFSIZE KB(4)
# endif

class Location
{
	private:
		std::string					root;
		std::vector<std::string>	indexes;
		std::vector<std::string>	methods;
		std::string					redir;
		std::string					e_is_dir;
		size_t						buf_size;

	public:
		Location(void);
		~Location();
};

#endif
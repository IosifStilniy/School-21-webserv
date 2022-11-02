#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <vector>

class Location
{
	private:
		std::string					root;
		std::vector<std::string>	indexes;
		std::vector<std::string>	methods;
		std::vector<std::string>	redirs;
		std::string					e_is_dir;

	public:
		Location(/* args */);
		~Location();
};

#endif
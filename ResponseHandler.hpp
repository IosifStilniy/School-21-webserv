#ifndef RESPONSEHANDLER_HPP
# define RESPONSEHANDLER_HPP

# include "Maintainer.hpp"

class ResponseHandler
{
	private:
	public:
		ResponseHandler(/* args */);
		~ResponseHandler();

		void	proceedResponse(Maintainer & maintainer);
};

#endif

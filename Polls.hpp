#ifndef POLLS_HPP
# define POLLS_HPP

# include <poll.h>
# include <stdexcept>
# include <string>
# include <cstring>
# include <sys/socket.h>

# define POLLCLR (POLLERR | POLLHUP | POLLNVAL)

class Polls
{
	private:
		size_t	_capacity;
		size_t	_current;
		size_t	_ready;

		Polls(void);

		void	_realloc(void);
		void	_erase(size_t i);

	public:
		pollfd *	polls;
		size_t		size;
		
		Polls(int socket, int flag);
		~Polls();

		pollfd *	operator[](int socket);

		void		append(int socket, int flag);
		pollfd &	back(void);
		void		setSockOpt(pollfd & poll_struct, int option_name, int option_value);
		void		poll(int timeout = -1);
		int			getNextSocket(void);
		size_t		getIndex(int socket)			const;
		void		clear(void);
};

#endif
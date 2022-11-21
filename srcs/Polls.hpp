#ifndef POLLS_HPP
# define POLLS_HPP

# include <poll.h>
# include <stdexcept>
# include <string>
# include <cstring>
# include <sys/socket.h>
# include <unistd.h>
# include <vector>

# define POLLCLR (POLLERR | POLLHUP | POLLNVAL)

class Polls
{
	private:
		size_t	_capacity;
		size_t	_current;
		int		_ready;


		void	_realloc(void);
		void	_erase(size_t i);

	public:
		pollfd *	polls;
		size_t		size;
		
		Polls(void);
		Polls(int * sockets, int * flags, int quantity);
		Polls(int socket, int flag);
		~Polls();

		pollfd *	operator[](int socket);

		void				append(int socket, int flag);
		pollfd &			back(void);
		void				setSockOpt(pollfd & poll_struct, int option_name, int option_value);
		int					poll(int timeout = -1);
		int					getNextSocket(void);
		size_t				getIndex(int socket)		const;
		bool				isListenSocket(int socket)	const;
		int					getListenSocket()			const;
		bool				isGoodByIndex(int indx)		const;
		bool				isReadyByIndex(int indx)	const;
		bool				isGood(int socket)			const;
		bool				isReady(int socket)			const;
		std::vector<int>	clear(void);
		void				purge(void);
};

#endif
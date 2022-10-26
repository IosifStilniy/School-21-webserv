#include "Polls.hpp"

Polls::Polls(void)
{
}

Polls::Polls(int socket, int flag): polls(new pollfd[2]), size(0), _capacity(2), _current(0), _ready(0)
{
	this->append(socket, flag);
}

Polls::~Polls()
{
	delete [] this->polls;
}

void	Polls::_realloc(void)
{
	this->_capacity *= 2;

	pollfd	*new_arr = new pollfd[this->_capacity];

	for (size_t i = 0; i < this->size; i++)
		new_arr[i] = this->polls[i];

	delete [] this->polls;
	this->polls = new_arr;
}

void	Polls::_erase(size_t i)
{
	this->size--;
	for (; i < this->size; i++)
		this->polls[i] = this->polls[i + 1];
}

pollfd *	Polls::operator[](int socket)
{
	for (size_t i = 0; i < this->size; i++)
		if (this->polls[i].fd == socket)
			return (this->polls + i);
	return (NULL);
}

void	Polls::append(int socket, int flag)
{
	if (this->size == this->_capacity)
		this->_realloc();

	if (socket < 0)
		throw std::runtime_error(strerror(errno));

	this->polls[this->size].fd = socket;
	this->polls[this->size].events = flag;
	this->polls[this->size++].revents = 0;
}

void	Polls::poll(int timeout = -1)
{
	this->_ready = ::poll(this->polls, this->size, timeout);
	this->clear();
}

int		Polls::getNextSocket(void)
{
	for (; this->_ready && this->_current < this->size; this->_current++)
	{
		if (!(this->polls[this->_current].events == this->polls[this->_current].revents))
			continue ;

		this->_ready--;
		return(this->polls[this->_current++].fd);
	}

	if (!this->_ready)
		this->_current = 0;

	return (0);
}

size_t	Polls::getIndex(int socket)	const
{
	for (size_t i = 0; i < this->size; i++)
		if (this->polls[i].fd == socket)
			return (i);
	
	return (-1);
}

void	Polls::clear(void)
{
	for (size_t i = this->size; i > 0; i--)
		if (POLLERR & this->polls[i - 1].revents)
			this->_erase(i - 1);
}
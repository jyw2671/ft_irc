#ifndef SOCKET_HPP
#define SOCKET_HPP

#include "log.hpp"
#include "client.hpp"
#include <fcntl.h>
#include <netdb.h>
#include <sys/event.h>
#include <unistd.h>
#include <arpa/inet.h>

#define IPV4_MAX 65535

class Socket
{
	public:
		typedef struct s_socket
		{
			int			fd;
			sockaddr_in	addr;
			socklen_t	len;
		} t_socket;

	private:
		void socket_create();
		void socket_bind(int port);
		void socket_listen();
		void socket_accept();
		void socket_close();
		Socket(const Socket&);
		Socket& operator=(const Socket&);

	protected:
		t_socket	_socket;
		ssize_t		_result;
		ssize_t		_remain;
		sockaddr_in	_addr;
		int			_fd;
		char		_buffer[IPV4_MAX];

		Socket();
		~Socket();
		void	init(int port);
		int		accept();
		ssize_t	receive(const struct kevent& evnet);
		ssize_t	send(const struct kevent& evnet);
		void	close(int fd);
};

#endif

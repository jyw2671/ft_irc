#ifndef IRCSOCKET_HPP
#define IRCSOCKET_HPP

#include "irclog.hpp"
#include <arpa/inet.h>

#define IPV4_MAX 65535

class IRCSocket
{
	public:
		/**
		 * @brief s_socket struct
		 *
		 * socket fd
		 * sockaddr_in struct
		 * socklen_t
		 */
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
		IRCSocket(const IRCSocket&);
		IRCSocket& operator=(const IRCSocket&);

	protected:
		t_socket	_socket;
		ssize_t		_result;
		ssize_t		_remain;
		//client 주소를 저장하기 위한 변수
		sockaddr_in	_addr;
		int			_fd;
		char		_buffer[IPV4_MAX];

		IRCSocket();
		~IRCSocket();
		void	init(int port);
		int		accept();
		ssize_t	receive(const struct kevent& evnet);
		ssize_t	send(const struct kevent& evnet);
		void	close(int fd);
};

#endif

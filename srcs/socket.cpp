#include "socket.hpp"

Socket::Socket()
{
	_socket.fd = -1;
	_socket.len = sizeof(_socket.addr);
}

Socket::~Socket()
{
}

void 	Socket::init(int port)
{
	socket_create();
	socket_bind(port);
	socket_listen();
}

//Socket create/bind/listen

void	Socket::socket_create()
{
	if ((_socket.fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		log::print() << "socket failed errno: " << errno << ":" << strerror(errno) << log::endl;
		exit(EXIT_FAILURE);
	}
	log::print() << "socket: " << _socket.fd << log::endl;
	int toggle = 1;
	setsockopt(_socket.fd, SOL_SOCKET, SO_REUSEPORT, (const void*)&toggle, sizeof(toggle));
}

void	Socket::socket_bind(int port)
{
	memset(&_socket.addr, 0, sizeof(sockaddr_in));
	_socket.addr.sin_family = AF_INET;
	_socket.addr.sin_addr.s_addr = INADDR_ANY;
	_socket.addr.sin_port = htons(port);

	if (bind(_socket.fd, (struct sockaddr*)&_socket.addr, sizeof(sockaddr_in)) == -1)
	{
		log::print() << "socket bind failed " << port << " port errno:" << errno << ":" << strerror(errno) << log::endl;
		exit(EXIT_FAILURE);
	}
	log::print() << "socket bind succeeded " << port << " ip " << inet_ntoa(_socket.addr.sin_addr) << log::endl;
}

void	Socket::socket_listen()
{
	if (listen(_socket.fd, SOMAXCONN) == -1)
	{
		log::print() << "socket listen failed " << errno << ":" << strerror(errno) << log::endl;
		exit(EXIT_FAILURE);
	}
	log::print() << "listen ot socket fd " << _socket.fd << log::endl;
}

void	Socket::socket_close()
{
	::close(_fd);
}

//Socket create/bind/listen/close fin

int		Socket::accept()
{
	if ((_fd = ::accept(_socket.fd, (sockaddr*)(&_addr), &_socket.len)) == -1)
		log::print() << "accpet failed errno: " << errno << ":" <<strerror(errno) << log::endl;
	else
		fcntl(_fd, F_SETFL, O_NONBLOCK);
	return (_fd);
}

ssize_t	Socket::receive(const struct kevent& event)
{
	_fd = event.ident;
	_result = ::recv(_fd, _buffer, event.data, 0);

	if (!_result)
		socket_close();
	return (_result);
}

ssize_t	Socket::send(const struct kevent& event)
{
	Client::t_to_client& to_client = ((Client*)event.udata)->get_buffers().to_client;
	_remain = to_client.buffer.size() - to_client.offset;
	_result = ::send(event.ident, to_client.buffer.data() + to_client.offset, event.data < _remain ? event.data : _remain, 0);
	return (_result);
}

void Socket::close(int fd)
{
	::close(fd);
}

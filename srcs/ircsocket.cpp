#include "ircsocket.hpp"

/**
 * @brief Construct a new IRCSocket::IRCSocket object
 *
 * what is socket?
 *
 * 소켓은 프로세스가 데이터를 내보내거나 데이터를 받기위한 실직적인 창구역할을 한다.
 * 프로세스가 데이터를 보내거나 받기위해서는 반드시 소켓을 열어서 보내거나 읽어야한다.
 *
 */
IRCSocket::IRCSocket()
{
	_socket.fd = -1;
	_socket.len = sizeof(_socket.addr);
}

IRCSocket::~IRCSocket()
{
}

/**
 * @brief sokcet 초기화
 *
 * 1. create : socket 생성
 * 2. bind : 생성한 socket을 server socket으로 등록
 * 3. listen : server sokcet을 통해 client의 접속 요청을 확인하도록 설정
 * 4. accept : client 접속 요청 대기 및 허락, client와 통신을 위해 새 socket 생성(client socket)
 * 5. read/write : client socket으로 데이터를 송수신
 * 6. close : client socket을 소멸
 * 6 -> 4 : 다른 client 접속 요청을 확인
 * @param port
 */
void 	IRCSocket::init(int port)
{
	socket_create();
	socket_bind(port);
	socket_listen();
}

/**
 * @brief socket 생성
 *
 * TCP/IP에서는 SOCK_STREAM을 UDP/IP에서는 SOCK_DGRAM을 사용
 *
 */
void	IRCSocket::socket_create()
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

/**
 * @brief socket bind
 *
 * 1. 만들어진 server_socket은 단지 socket 디스크립터일 뿐이다.
 * 2. 이 socket에 주소를 할당하고 port번호를 할당해서 커널에 등록해야한다.
 * 3. 커널에 등록해서 다른 시스템과 통신할 수 있는 상태가 된다.
 * 4. 커널이 socket을 이용해 외부로부터 자료를 수신할 수 있게 되는 것이다.
 * 5. socket에 주소와 port번호를 할당하기 위해서 sockaddr_in 구조체를 이용한다.
 * 6. htonl/htons 주소를 지정해주는 함수. 시스템마다 IP가 다르기 때문에 주소 지정을 고정 IP로 하지않고 사용한다.
 * @param port
 */
void	IRCSocket::socket_bind(int port)
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

/**
 * @brief socket listen
 *
 * client 접속 요청을 확인
 */
void	IRCSocket::socket_listen()
{
	if (listen(_socket.fd, SOMAXCONN) == -1)
	{
		log::print() << "socket listen failed " << errno << ":" << strerror(errno) << log::endl;
		exit(EXIT_FAILURE);
	}
	log::print() << "listen ot socket fd " << _socket.fd << log::endl;
}

/**
 * @brief socket close
 *
 */
void	IRCSocket::socket_close()
{
	::close(_fd);
}

/**
 * @brief accept()
 *
 * 1. accept()로 접속요청을 허락하게되면 client와 통신을 하기위해서 커널이 자동으로 socket(client socket)을 생성한다.
 * 2. client socket 정보를 구하기 위해서 변수를 선언하고 client 주소 크기를 대입한다.
 *
 * @return client socket 디스크립터
 */
int		IRCSocket::accept()
{
	if ((_fd = ::accept(_socket.fd, (sockaddr*)(&_addr), &_socket.len)) == -1)
		log::print() << "accpet failed errno: " << errno << ":" <<strerror(errno) << log::endl;
	else
		fcntl(_fd, F_SETFL, O_NONBLOCK);
	return (_fd);
}

/**
 * @brief receive
 *
 * 1. recv()를 이용하여 client로 부터 전송된 자료를 읽어들인다.
 * 2. client로 부터 전송된 자료가 없다면 송신할 때까지 대기하게 된다. block된 상태
 * -> 수정필요
 *
 * recv() function
 * @param event
 * @return ssize_t
 */
ssize_t	IRCSocket::receive(const struct kevent& event)
{
	_fd = event.ident;
	_result = ::recv(_fd, _buffer, event.data, 0);
	if (!_result)
		socket_close();
	return (_result);
}

/**
 * @brief send
 *
 * send() function
 * @param event
 * @return ssize_t
 */
ssize_t	IRCSocket::send(const struct kevent& event)
{
	IRCClient::t_to_client& to_client = ((IRCClient*)event.udata)->get_buffers().to_client;
	_remain = to_client.buffer.size() - to_client.offset;
	_result = ::send(event.ident, to_client.buffer.data() + to_client.offset, event.data < _remain ? event.data : _remain, 0);
	return (_result);
}

void IRCSocket::close(int fd)
{
	::close(fd);
}

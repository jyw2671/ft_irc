#include "../includes/ircsocket.hpp"
#include "../includes/ircclient.hpp"
#include <fcntl.h>
#include <netdb.h>
#include <sys/event.h>
#include <unistd.h>

/**
 * @brief Construct a new IRCSocket::IRCSocket object
 *
 * what is socket?
 *
 * 소켓은 프로세스가 데이터를 내보내거나 데이터를 받기위한 실직적인 창구역할을 한다.
 * 프로세스가 데이터를 보내거나 받기위해서는 반드시 소켓을 열어서 보내거나 읽어야한다.
 *
 * - 두 프로그램이 서로 데이터를 주고 받을 수 있도록 양쪽에 생성되는 통신단자
 * - 프로세스가 데이터를 내보내거나 받기위한 실직적인 창구역할을 하며 데이터를 내보내고 받기위해선 반드시 소켓을 열어 소켓에 데이터를 써보내거나 소켓을 통해 데이터를 읽어야한다.
 * - 소켓은 프로토콜, IP, port로 정의된다.
 * - 프로토콜 : 어떤 시스템이 다른 시스템과 통신을 원활하게 수용하도록 해주는 통신 규약, 약속
 * - IP : 전 세계 컴퓨터에 부여된 고유의 식별 주소
 * - port : 네트워크 상에서 통신하기 위해 호스트 내부적으로 프로세스가 할당받아야 하는 고유한 숫자.
 *      한 호스트 내에서 네트워크 통신을 하고 있는 프로세스를 식별하기 위해 사용되는 값으로, 같은 호스트 내에서 서로 다른 프로세스가 같은 포트넘버를 가질 수 없다.
 *      같은 컴퓨터 내에서 프로그램을 식별하는 번호.
 * - 즉, 소켓은 떨어져 있는 두 호스트를 연결해주는 도구로서 인터페이스의 역할을 한다. 데이터를 주고 받을 수 있는 구조체로 소켓을 통해 테이터 통로가 만들어진다.
 */

 /**
소켓 통신이란?
- 클라이언트와 서버 양쪽에서 서로에게 데이터를 전달하는 양방향 통신
- 서버와 클라이언트 양방향 연결이 이루어지는 통신으로, 클라이언트도 서버로 요청을 보낼 수 있고 서버도 클라이언트로 요청을 보낼 수 있다.
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
 * @brief server sokcet 초기화
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

/**
 * CLIENT SOCKET
 * 실제로 데이터 송수신이 일어나는 것은 클라이언트 소켓이다
 * 1.socket() function으로 socket 생성
 * 2.connect() function을 이용하여 통신할 서버의 설정된 ip와 port번호에 통신을 시도
 * 3.통신 시도 시, 서버가 accpet() 함수를 이용하여 클라이언트의 socket descriptor를 반환
 * 4.이를 통해 클라이언트와 서버가 서로 read()/write()를 하며 통신
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
	/**
	 * @brief socket function
	 * int socket(int domain, int type, int protocol)
	 * header
	 * 	- sys/type.h, sys/socket.h를 포함해야한다.
	 * domain
	 * 	- 어떤 영역에서 통신할 것인지에 대한 영역 지정
	 * 	- 프로토콜 family를 지정해주는 것
	 * 	- AF_UNIX(프로토콜 내부), AF_INET(IPv4), AF_INET6(IPv6)
	 * type
	 * 	- 어떤 프로토콜을 사용할 것인지에 대한 설정
	 * 	- 데이터 전송형태 지정
	 * 	- SOCK_STREAM(TCP), SOCK_DGRAM(UDP), SOCK_RAW(사용자 정의)
	 * 	- SOCK_STREAM(TCP)
	 * 		- 연결지향형
	 * 		- 전송한 순서대로 data가 보내진다.
	 * 		- 중간에 data가 소멸되지 않고 목적지에 잘 전송된다.
	 * 		- 데이터의 경계가 존재하지 않는다.
	 * 	- SOCK_DGRAM(UDP)
	 * 		- 비연결지향형, 속도 우선
	 * 		- 전송 순서에 관계없이 빠른 전송을 지향한다.
	 * 		- data 손실 또는 파손의 우려가 있다.
	 * 		- 한번에 전송할 수 있는 data 크기가 제한된다.
	 * 		- 데이터의 경계가 존재한다. (5번에 나눠 100byte를 전송하면 받는사람도 5번에 나눠 100byte를 받아야한다.)
	 * protocol
	 * 	- 어떤 프로토콜의 값을 결정하는 것
	 * 	- 통신에 있어 특정 프로토콜을 사용을 지정하기 위한 변수
	 * 	- 보통 첫번째와 두번째 인자만 전달해도 어떤 프로토콜을 사용할 것인지 알 수 있어 주로 0을 사용
	 * 	- 첫번째와 두번째 인자을 전달해도 다양한 프로토콜이 있을 수 있어 사용에 따라 변수 지정
	 * 	- 0, IPPROTO_TCP(TCP의 경우), IPPROTO_UDP(UDP의 경우)
	 * return value
	 * - -1 실패
	 * - socket discriptor
	 */
	if ((_socket.fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		log::print() << "socket failed errno: " << errno << ":" << strerror(errno) << log::endl;
		exit(EXIT_FAILURE);
	}
	log::print() << "socket: " << _socket.fd << log::endl;

	//Forcefully attaching socket to the port
	/**
	 * setsockopt function
	 * int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen)
	 * 생성된 socket에 속성값을 변경하는 함수
	 *
	 * header
	 * - sys/socket.
	 *
	 * sockfd
	 * 	- socket, accpet 등으로 생성된 소켓 디스크립터
	 * level
	 * 	- optname값이 socket level인지 특정 protocol에 대한 설정인지 지정하는 값
	 * 	- SOL_SOCKET
	 * 		- optname이 socket level에서 설정하는 option명임을 지정
	 * 	- IPPROTO_IP
	 * 		- optname이 IP protocol level에서 설정하는 option명임을 지정
	 * 	- IPPROTO_TCP
	 * 		- optname이 TCP protocol level에서 설정하는 option명임을 지정
	 * optname
	 * 	- protocol level에 따른 option 종류 참고 자료 https://jhnyang.tistory.com/262
	 * optval
	 * 	- optname에 따른 설정할 값
	*/

	/**
	 * 소켓 세부 설정이 필요한가?
	 * 통신이 하나의 서버에 하나의 클라이언트만으로 이뤄지는 것은 아니며, 하나의 트랜잭션만 발생하는 것이 야니다.
	 * 그렇기 때문에 생각하지 못한 상황이 발생할 수도 있으며, 네크워크 환경을 모두 예측하기 힘들다.
	 * 보통 기본으로 설정되어 있어 세부 설정을 할 일이 없을 것 같지만, 세부설정을 해줘야하는 상황이 꽤 흔하게 발생한다.
	*/

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
	/**
	 * 소켓의 통신의 대상을 지정하기 위해 주소를 사용
	 * 주소를 저장하거나 표현하는데 사용하는 구조체가 struct sockaddr이다.
	 * bind( ), connect( ) 와 같은 함수들이 2번째 매개변수로써 바로 이 struct sockaddr을 받는다.
	 * struct sockaddr은 기본형태이고, struct sockaddr_in, struct sockaddr_un, struct sockaddr_in6 등의 구조체와 상호 형변환을 해서 사용하게 된다.
	 * 주소 체계는 AF_INET, AF_INET6, AF_UNIX, AF_LOCAL, AF_LINK, AF_PACKET 등이 있다.
	 *
	 * struct sockaddr {
	 * 	u_short    sa_family;     // address family, 2 bytes
	 * 	char    sa_data[14];     // IP address + Port number, 14 bytes
	 * };
	 * sa_family : 주소체계를 구분하기위한 변수, 2bytes.
	 * sa_data : 실제 주소를 저장하기위한 변수, 14bytes.
	*/

	/**
	 * struct sockaddr_in
	 * - sa_family가AF_INET(IPv4)인 경우
	 * - sockaddr을 그래도 사용할 경우, sa_data에 IP주소 port번호가 조합되어 있어 쓰거나 읽기 불편하다.
	 * struct sockaddr_in {
	 * 	short    sin_family;          // 주소 체계: AF_INET, 2byte
	 * 	u_short  sin_port;            // 16 비트 포트 번호, network byte order, 2byte
	 * 	struct   in_addr  sin_addr;   // 32 비트 IP 주소, 4byte
	 * 	char     sin_zero[8];         // 전체 크기를 16bytes로 맞추기 위한 dummy, 8byte
	 * };
	 *
	 * struct  in_addr {
	 * 	u_long  s_addr;     // 32비트 IP 주소를 저장 할 구조체, network byte order
	 * };
	 * 	참고 : https://techlog.gurucat.net/292
	 * struct sockaddr_in serv_addr = {};
	 * - Clear address structure, should prevent som segmentation fault and artifacts
	 * - sin_zero : 8bytes dummy data, 반드시 0으로 채워져 있어야한다.
	*/

	/**
	 * htons()
	 * - header : arpa/inet.h
	 * - convert unsigned short int to big-endian network byte oreder as expected from TCP protocol standards
	 * - short 메모리 값을 호스트 바이트 순서에서 네트워크 바이트 순서로 변경한다.
	 * - 호스트 바이트 순서는 지금 사용중인 시스템에서 2바이트 이상의 큰 숫자 변수에 대해 바이트를 메모리 상에 어떻게 배치하는지 순서를 말한다.
	 * - 네트워크 호스트 바이트 순서는 2바이트 이상의 큰 숫자 변수에 대해 어떤 바이트부터 전송할지에 대한 순서를 말한다.
	 * - 2바이트 이상 큰 숫자를 전송할 때, 큰 단위를 먼저 전송할지 작은 단위를 먼저 전송할지 설정하지 않으면 문제가 발생한다.
	 * - 네트워크에서 big-endian을 사용한다. big-endian을 사용하는 시스템은 그대로 전송하면 되지만 little-endian을 사용하는 호스트에서는 바이트 순서를 바꿔서 전송해야한다.
	 * - 코드를 통해 맞춘다면 시스템에 독립적인 프로그램을 작성할 수 없기때문에 htons 함수를 사용한다.
	*/
	memset(&_socket.addr, 0, sizeof(sockaddr_in));
	_socket.addr.sin_family = AF_INET;
	_socket.addr.sin_addr.s_addr = INADDR_ANY;
	_socket.addr.sin_port = htons(port);

	//Bind the socket to the current IP address on selected port
	/**
	 * bind()
	 * - 소켓에 IP주소와 port번호를 지정하여 소켓을 통신에 사용할 수 있도록 준비한다.
	 * - int bind(int sockfd, struct sockaddr *myaddr, socklen_t addrlen)
	 * - header : sys/type.h, sys/socket.h
	 * - sockfd : 소켓 디스크립터
	 * - *myaddr : 주소 정보로 인터넷을 이용하는 AF_INET인지 시스템 내에서 통신하는 AF_UNIX에 따라 달라진다.
	 * - addrlen : myaddr 구조체의 크기
	 * - return value
	 * 	- 성공 시 : 0
	 * 	- 실패 시 : -1
	 * */
	if (bind(_socket.fd, (struct sockaddr*)&_socket.addr, sizeof(sockaddr_in)) == -1)
	{
		log::print() << "socket bind failed " << port << " port errno:" << errno << ":" << strerror(errno) << log::endl;
		exit(EXIT_FAILURE);
	}
	/**
	 * inet_ntoa
	 * - char *inet_ntoa(struct in_addr in)
	 * - network byte order의 주소를 a.b.c.d 형태인 IPv4 주소 형태의 문자열로 반환합니다.
	 * - struct in_addr은 AF_INET address family 주소 체계에 포함된 구조입니다.
	 * - accept, getpeername, getsockname 등으로 얻은 주소 정보입니다.
	 * - return
	 * 	- a.b.c.d와 같은 IPv4 주소
	 * 	- 숫자 점 표기법의 IP주소
	*/
	log::print() << "socket bind succeeded " << port << " ip " << inet_ntoa(_socket.addr.sin_addr) << log::endl;
}

/**
 * @brief socket listen
 *
 * client 접속 요청을 확인
 */
void	IRCSocket::socket_listen()
{
	//Let socket be able to listen for requsets
	/**
	 * listen()
	 * - SOCK_STREAM, SOCK_SEQPACKET 등 연결지향형 socket에 대해 server socket을 활성화하여 client의 접속을 허용하게 한다.
	 * - client의 접속 요청에 대해 accept를 통해 연결이 맺어지고 accept되지 못한 요청은 backlog의 크기만큼 queue에 쌓인다.
	 * - backlog의 크기보다 많은 connection이 쌓이면 client는 ECONNREFUSED 오류가 발생한다.
	 *
	 * - int listen(int sockfd, int backlog)
	 * - header : sys/type.h, sys/socket.h
	 * - sockfd : 소켓 디스크립터, 해당 디스크립터를 생성하는 socket 함수의 두번째 파라미터인 type이 OCK_STREAM, SOCK_SEQPACKET과 같은 연결지향형이어야 한다.
	 * - balcklog : client의 접속 요청에 대해 accept를 통하여 양쪽 socket에 연결되는데, accept가 빨리 되지 않을 때 대기할 queue의 개수.
	 * - return value
	 * 	- 정상 동작 : -1이 아닌 값
	 * 	- 오류 발생 : -1
	 * 	- 자세한 내용은 errno에 저장된다.
	 * - client의 접속 요청이 많은 경우, 대기하지않고 빠른 accept를 사용할 수 있도록 multi-process, multi-thread 등의 방식을 사용한다.
	 *
	 * https://www.it-note.kr/115
	*/
	if (listen(_socket.fd, SOMAXCONN) == -1)
	{
		log::print() << "socket listen failed " << errno << ":" << strerror(errno) << log::endl;
		exit(EXIT_FAILURE);
	}
	log::print() << "listen ot socket fd " << _socket.fd << log::endl;
	fcntl(_socket.fd, F_SETFL, O_NONBLOCK);
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
 *
 * Client의 접속 요청을 기다리고 요청이 오면 client와 연결한다.
 *
 * 1. accept()로 접속요청을 허락하게되면 client와 통신을 하기위해서 커널이 자동으로 socket(client socket)을 생성한다.
 * 2. client socket 정보를 구하기 위해서 변수를 선언하고 client 주소 크기를 대입한다.
 * @return client socket 디스크립터
 */
int		IRCSocket::accept()
{
	/**
	 * accept()
	 * - client의 접속 요청을 받아들여 client와 연결한다. accept함수의 결과로 client와 연결을 유지하는 새로운 socket을 생성한다.
	 * - socket() 호출 시 type이 SOCK_STREAM, SOCK_SEQPACKET과 같은 연결지향성 소켓이어야 합니다.
	 * - int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
	 * - sockfd : 소켓 디스크립터, bind 및 listen이 완료된 상태
	 * - *addr
	 * 	- 접속한 client의 주소정보를 저장.(설정하는 값이 아니라 얻는 값이다)
	 * 	- addr에 NULL을 넘기면 client의 주소 정보를 받지 않겠다는 의미
	 * - addrlen
	 * 	- addr의 크기를 설정
	 * 	- 접속이 완료되면 실제로 addr에 설정된 접속한 client의 주소 정보 크기를 저장
	 * 	- addr에 NULL을 넘기면 addrlen의 값은 무시된다.
	 * - return value
	 * 	- 정상 동작 : -1이 아닌 값, client와 연결된 새로운 socket이 생성된다.
	 * 	- 오류 발생 : -1
	 * 	- 자세한 내용은 errno에 저장된다.
	 * - server socket은 socket()을 통하여 생성된 소켓 디스크립터, client와 통신할 수 없으며, accept가 return한 socket()을 통하여 client와 데이터를 송수신할 수 있다.
	 * https://www.it-note.kr/116?category=1068194
	*/
	if ((_fd = ::accept(_socket.fd, (sockaddr*)(&_addr), &_socket.len)) == -1)
		log::print() << "accpet failed errno: " << errno << ":" <<strerror(errno) << log::endl;
	else
		fcntl(_fd, F_SETFL, O_NONBLOCK);
	/**
	 * 	As requested from subject we set the socket to NON-BLOCKING mode
	 * 	allowing it to return any data that the system has in it's read buffer
	 * 	for that socket, but it won't wait for that data
	*/
	/**
	 * @brief fcntl
	 *
	 * - int fcntl(int fd, int cmd, long arg)
	 * - fcntl을 사용해서 할 수 있는 일은 cmd로 결정됩니다.
	 * - 과제에서 요구하는 fcntl의 사용법은 fcntl(fd, F_SETFL, O_NONBLOCK)로 정해져 있습니다.
	 * - 이 이외의 인자를 사용하면 0점
	 *
	 * - F_SETFL : 파일 지정자에 대한 값을 설정
	 * - O_NONBLOCK : fd를 NON_BLOCK 모드로 설정
	 *  - NON_BLOCK으로 fd를 변경하게 되면 read/write/send/recv/connect 등 기본적으로 BLOCK으로 사용되던 함수들이 NON_BLOCK으로 동작합니다.
	 *
	 * - read/recv
	 *  - BLOCK : read 버퍼가 비었을 때 block
	 *  - NON_BLOCK : read 버퍼가 비었을 때 -1을 반환, errno는 EWOULDBLOCK/EAGAIN 설정
	 *
	 * - write/send
	 *  - BLOCK : write 버퍼가 비었을 때 block
	 *  - NON_BLOCK : write 버퍼가 비었을 때 -1을 반환, errno는 EWOULDBLOCK/EAGAIN 설정
	 *
	 * - accept
	 *  - BLOCK : backlog(현재 connection 요청 큐)가 비었을 때 block
	 *  - NON_BLOCK : backlog(현재 connection 요청 큐)가 비었을 때 -1을 반환, errno는 EWOULDBLOCK/EAGAIN 설정
	 *
	 * - connect
	 *  - BLOCK : connection이 완전히 이루어질 때 까지 block
	 *  - NON_BLOCK :connection이 완전히 이루어지지 않아도 return되며, -1의 값을 반환하고 나중에 getsockopt로 확인할 수 있다. (getsockopt는 허용함수가 아니다). errno는 EWOULDBLOCK/EAGAIN 설정
	 */

	return (_fd);
}

/**
 * @brief receive
 *
 * 1. recv()를 이용하여 client로 부터 전송된 자료를 읽어들인다.
 * 2. client로 부터 전송된 자료가 없다면 수신할 때까지 대기하게 된다. block된 상태
 *
 * recv() function
 * @param event
 * @return ssize_t
 */
ssize_t	IRCSocket::receive(const struct kevent& event)
{
	/**
	 * recv()
	 * - ssize_t recv(int sockfd, void *buf, size_t len, int flags)
	 * - sockfd : connect, accept로 연결된 socket descriptor
	 * - buf : data를 수신하여 저장할 버퍼
	 * - len : 읽을 데이터의 크기
	 * - flags : 읽을 데이터 유형 또는 읽는 방법에 대한 옵션
	 * 	- MSG_OOB : out of band(긴급 데이터) 데이터를 읽음, 주로 X.25에서 접속이 끊겼을 때 전송되는 데이터
	 * 	- MSG_PRRK : receive queue의 데이터를 queue에서 제거하지 않고 확인하기 위한 목적으로 설정
	 * 	- MSG_WAILALL : 읽으려는 데이터가 버퍼에 찰 때까지 대기
	 * 	- 0 : 일반 데이터를 수신하고 read와 같은 동작을 합니다.
	 * - return
	 * 	- 성공 시 : 0이상, - 정상적으로 데이터를 수신하였으며, 실제로 수신한 데이터의 길이를 return합니다.
	 * 	- 실패 시 : -1, 오류 발생, 상세한 오류 내용은 errno에 저장됩니다.
	*/
	_fd = event.ident;
	_result = ::recv(_fd, _buffer, event.data, 0);
	/**
	 * @brief
	 * client 쪽에서 전송한 데이터가 없으면 socket_close
	 * 서버는 종료하지않고 다른 event처리, 즉 다른 client와 통신을 위해 대기합니다.
	 */
	if (!_result)
		socket_close();
	return (_result);
}

/**
 * @brief send
 *
 * 1. 수신된 데이터의 길이를 구하여 전송데이터를 준비합니다.
 * 2. send(write)를 이용하여 client로 자료를 송신합니다.
 * send() function
 * @param event
 * @return ssize_t
 */
ssize_t	IRCSocket::send(const struct kevent& event)
{
	/**
	 * @brief send()
	 *
	 * ssize_t send(int sockfd, const void *buf, size_t len, int flags)
	 * - sockfd : connect, accept로 연결된 socket descriptor
	 * - buf : 전송할 데이터를 저장할 버터
	 * - len : 전송할 데이터의 길이
	 * - flags
	 *  - 전송할 데이터 또는 읽는 방법에 대한 옵션, 0 또는 bit 연산으로 설정가능
	 *  - MSG_DONTTROUTE : gateway를 통하지 않고 직접 상대에게 전달
	 *  - MSG_DONTWAIT : nonblocking에서 사용하는 옵션, 전송이 block되면 EAGIN, EWOULDBLOCK 오류로 바로 반환
	 *  - MSG_MORE : 전송할 데이터가 남아있음을 설정
	 *  - MSG_OOB : out of band 데이터를 읽습니다. 주로 X.25에서 접속이 끊겼을 때에 전송되는 데이터 flags 값이 0이면 일반 데이터를 전송하며, write를 호출한 것과 같습니다.
	 *  - flag를 설정하지 않는 경우 write와 같은 동작을 합니다.
	 * - return
	 *  - 성공 시 : 0이상, - 정상적으로 데이터를 전송하였으며, 실제로 전송된 데이터의 길이를 return합니다.
	 *  - 실패 시 : -1, 오류 발생, 상세한 오류 내용은 errno에 저장됩니다.
	 */
	IRCClient::t_to_client& to_client = ((IRCClient*)event.udata)->get_buffers().to_client;
	_remain = to_client.buffer.size() - to_client.offset;
	_result = ::send(event.ident, to_client.buffer.data() + to_client.offset, event.data < _remain ? event.data : _remain, 0);
	return (_result);
}

/**
 * @brief 채널에서 해당 세션을 닫는 처리를 합니다.
 *
 * @details 여기에서 세션 맵에 있는 세션의 공유포인터가 std::map의 erase 메서드로 삭제됩니다.
 * 이후, 공유 포인터에 저장된 세션의 실제 포인터가 참조 카운트에 따라 자동으로 삭제될 것입니다.
 *
 * 참조 카운트에 따라 자동으로 삭제되면 해당 소멸자에서 소켓 fd의 close 및 kqueue에서 이벤트 등록해제 등이 모두 이루어집니다.
 *
 * @param session 세션을 닫는 처리를 할 Session 포인터 입니다.
 *
 * @exception 이벤트 해제가 일부라도 실패할 경우 std::runtime_error 예외가 발생합니다.
 */
void IRCSocket::close(int fd)
{
	::close(fd);
}

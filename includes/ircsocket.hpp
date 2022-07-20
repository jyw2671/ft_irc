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

		/**
	     * @brief 연결을 수락할 때 사용하는 함수입니다.
	     * @details 이 함수를 실행하여서 backlog에 있는 socket을 accept를 사용해 꺼내오고,
	     * 새로운 세션 인스턴스를 만든 후 세션 맵에 넣고, 해당 소켓에 대해 통신을 할 수 있게 IO이벤트를 kqueue에 등록합니다.
	     *
	     * Accept에 실패해도 예외가 발생하지는 않으며, 단순히 accept에 실패했다는 오류 메시지만 나옵니다.
	     * 따라서 오류가 발생한 세션을 간단히 종료시키고, 계속해서 다른 연결을 받을 수 있습니다.
	     */
		int		accept();

		/**
	     * @brief 세션에서 데이터를 읽어올 때 사용하는 함수입니다.
	     * @details kevent 함수에서 데이터를 읽을 수 있음이 확인되면,
	     * 이 함수를 호출해서 시스템 커널에 저장된 데이터를 읽어 세션의 _recvBuffer에 저장합니다.
	     * 이후, <CR><LF>를 구분자로 하여 내용을 구분하고, 분석하여 처리할 수 있게 합니다.
	     * 처리된 데이터는 한 줄씩 Session::Process 함수를 호출하여 넘깁니다.
	     *
	     * 내부의  함수 호출에 실패하여 문제가 발생하면, Close 함수가 호출되어 연결을 종료하게 됩니다.
	     *
	     * @param session 데이터를 읽을 Session 포인터 입니다.
	     *
	     * @exception session이 NULL이라면 std::runtime_error 예외가 발생합니다.
	     */
		ssize_t	receive(const struct kevent& evnet);

		/**
	     * @brief 세션에서 원격으로 데이터를 보낼 때 사용하는 함수입니다.
	     * @details kevent 함수에서 데이터를 1 바이트라도 보낼 수 있음이 확인되면, 이 함수를 호출해서
	     * 세션의 _sendBuffer에서 데이터를 꺼내 시스템 커널에 원격으로 데이터 전송을 요청합니다.
	     *
	     * 보내진 데이터는 _sendBuffer에서 삭제되어 꺼내지며(Polling), 더 이상 보낼 데이터가 없다면
	     * 쓰기 이벤트를 비활성화 합니다. 추후 쓰기 이벤트가 다시 활성화 되면, 이 함수가 다시 호출될 수 있습니다.
	     *
	     * 내부의 send 함수 호출에 실패하여 문제가 발생하면, Close 함수가 호출되어 연결을 종료하게 됩니다.
	     *
	     * @param session 데이터를 작성할 Session 포인터 입니다.
	     *
	     * @exception session이 NULL이라면 std::runtime_error 예외가 발생합니다.
	     */
		ssize_t	send(const struct kevent& evnet);
		void	close(int fd);
};

#endif

#ifndef IRCEVENT_HPP
#define IRCEVENT_HPP

#include "ircclient.hpp"
#include "irclog.hpp"
#include <sys/event.h>
#include <sys/types.h>

/**
I/O multiplexing
I/O blocking이 발생해서 여러개의 다른 클라이언트에게 접속을 허용할 수 없거나 성능 저하가 발생하는 경우가 생긴다.

이를 해결하기 위한 방법들로는
1.Fork : 프로세스를 새로 만드는 방법으로 클라이언트 요청이 있을때마다 프로세스를 복사하여 여러 사용자에게 서비스를 제공
2.Thread : 프로세스 방법이 아닌 thread를 생성해서 여러 사용자들에게 서비스를 제공
3.I/O Multiplexing : 여러 소켓에 대해 I/O를 병행적으로 처리하는 기법. 다수의 프로세스 혹은 thread를 만들지 않고도 여러 파일을 처리할 수 있다.
4.less memory, less context swithing이 가능해 개선된 성능을 보여준다.
5.비동기 I/O : I/O가 비동기적으로 처리되는 기법. 시그널이 병행되어 존재한다.
6.Event Driven I/O : I/O multiplexing을 추상화

I/O multiplexing을 구현하기 위한 시스템 호출로 select, poll, epoll, kqueue 등이 있다.
I/O multiplexing 기능은 프로그램에서 여러 파일 피스크립터를 모니터링해서 어떤 종류의 I/O 이벤트가 발생했는지 검사하고
각각의 파일 디스크립터가 ready 상태가 되었는지 확인하는 것이 주요 목표이다.

select
등록된 파일 디스크립터를 하나하나 체크해서 커널과 유저 공간 사이에 여러번의 데이터 복사를 진행
파일 디스크립터를 하나하나 체크해야 하므로 O(n)의 계산량이 필요하고 관리하는 파일 디스크립터의 수가 증가하면 성능이 떨어진다.
관리할 수 있는 파일 디스크립터 수에 제한이 있다.
사용이 쉽고 지원하는 OS가 많아 이식성이 좋다.

poll
select와 거의 동일하지만 관리할 수 있는 파일 디스크립터가 무제한이다.
low level의 처리로 system call의 호출이 select보다 적고 이식성이 좋지 않다.
접속 수가 늘어나면 오히려 fd당 체크 마스크의 크기가 select는 3bit이지만, poll은 64bit정도로 양이 많이지면 성능이 select보다 떨어진다.
*/
class IRCClient;

class IRCEvent
{
	private:
		IRCEvent(const IRCEvent&);
		IRCEvent& operator=(const IRCEvent&);

		//EV_SET(&kev, ident, filter, flags, fflags, data, udata);
		void event_set(uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void *udata);
		int _kqueue;
		int _count;

	protected:
		struct kevent _events[EVENTS_MAX];
		int		_index;

		IRCEvent();
		~IRCEvent();

		int		kevent();
		void	init(int socket_fd);
		void	add(IRCClient*);
		void	remove(int fd);
		void	toggle(int EVFILT_TYPE);
		void	toggle(IRCClient&, int EVFILT_TYPE);
};

#endif

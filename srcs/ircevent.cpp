#include "../includes/ircevent.hpp"

/**
 * @brief Construct a new IRCEvent::IRCEvent object
 *
 * What is kqueue?
 * - BSD 계열에서 지원하는 Event관리 system call로, linux 계열에서 select를 개선한 epoll과 비슷하게 사용되고 동작한다.
 * - 여러 fd를 모니터링하고, fd에 대한 동작(read/recv, write/send)이 준비되었는지 알아내는데 사용되어 I/O multiplexing을 이용하는 서버 프로그램을 작성하는데 사용된다.
 * - kqueue는 커널에 할당된 폴링 공간(kernel event queue-kqueue)dp 모니터링할 이벤트를 등록하고,
 * - 발생한 이벤트를 return 받아 multiplexing I/O event를 처리할 수 있도록 도와준다.
 * - 이벤트 등록 및 반환은 kevent 구조체를 통해 이뤄지며, 구조체 필드로 존재하는 이벤트에 대한 필터, 플래그 등을 이용해
 * - 다양한 이벤트 발생 상황에 대한 정의 및 발생 이벤트에 대한 정보를 확인할 수 있다.
 *
 * kqueue()
 * - kqueue()는 커널에 새로운 event queue를 만들고, fd를 반환한다.
 * - 반환된 fd는 kevent()에서 이벤트를 등록, 모니터링하는데 사용된다.
 * - queue는 fork(2)로 자식 프로세스 분기 시 사용되지 않는다.
 * - header
 * 	- #include <sys/time.h>
 * 	- #include <sys/event.h>
 * 	- #include <sys/types.h>
 * - function
 * 	- int kqueue(void)
 * - return value
 * 	- fd
 *
 * kevent()
 * int kevent(int kq, const struct kevent *changelist, int nchanges,
 * 			struct kevent *eventlist, int nevents,
 * 			const struct timespec *timeout)
 * - kq : kqueue에 새로 모니터링할 이벤트를 등록
 * - changelist : 는 kevent 구조체의 배열로 changelist 배열에 저장된 kevent 구조체(이벤트)들은 kqueue에 저장된다.
 * - nchanges : 등록할 이벤트의 개수이다.
 * - eventlist : 발생한 event가 반환될 배열.
 * - nevents : eventlist 배열의 크기. kevent()는 이 배열에 발생한 kevent를 최대 nevents만큼 정리하여 담아주고, 그 개수를 반환한다.
 * 		- changelist와 eventlist는 같은 배열을 사용할 수 있는데, 관리의 용이성을 위해 따로 둘 수 있다.
 * - timeout : timespec 구조체의 포인터를 전달하고, NULL을 전달할 경우 이벤트 발생까지 block된다. (무한정 대기)
 * - return
 * 	- 발생하여 아직 처리되지 않은(pending 상태인) 이벤트의 개수
 *
 * kevent struct
 * - header
 * 	- #include <sys/event.h>
 * struct kevent {
 *	uintptr_t ident;        identifier for this event
 *	int16_t   filter;       filter for event
 *	uint16_t  flags;        action flags for kqueue
 *	uint32_t  fflags;       filter flag value
 *	intptr_t  data;         filter data value
 *	void      *udata;       opaque user data identifier
 *	};
 *
 * - ident : event에 대한 식별자, fd번호가 입력된다.
 * - filter : 커널이 event를 핸들링할 때 이용되는 필터이다.filter 또한 event의 식별자로 사용.
 * 	- 주로 사용되는 filter
 * 		- EVFILT_READ : ident의 fd를 모니터링하고 읽을 수 있는 data가 있다면(read가 가능한 경우) event는 반환된다. file descriptor의 종류에 따라 조금씩 다른 종작을 한다.(socket, vnodes, fifo, pipe etc.)
 * 		- EVFILT_WRITE : ident의 fd를 모니터링하고 write가 가능한 경우 event가 반환된다.
 * 		- EVFILT_VNODE, EVFILT_SIGNAL 등이 있다.
 * - flag : event를 적용시키거나 event가 반환되었을 때 사용되는 flag.
 * 	- 주로 사용되는 flag
 * 		- EV_ADD : kqueue에 event를 등록한다. 이미 등록된 event(ident, filter), 즉 식별자가 같은 event를 재등록하는 경우 새로 만들지않고 덮어쓴다.등록된 event는 자동으로 활성화된다.
 * 		- EV_ENABLE : kevent()가 event를 반환할 수 있도록 활성화한다.
 * 		- EV_DISABLE : kevent()가 event 반환하지 않도록 한다. event를 비활성화 한다.
 * 		- EV_DELETE : kqueue에서 event를 삭제한다. fd를 close()한 경우 관련 event는 자동으로 삭제된다.
 * - fflags : filter에 따라 다르게 적용되는 flag이다.
 * - data : filter에 따라 다르게 적용되는 data값이다.
 * 	- EVFILT_READ인 경우 event가 반환되었을 때 data에는 read가 간으한 바이트 수가 기록된다.
 * - udata : event와 함께 등록하여 event 반환시 사용할 수 있는 user-data이다. udata 또한 event의 식별자로 사용될 수 있다. (optional - kevent64() 및 kevent_qos()는 인자 flags로 udata를 식별자로 사용할지 말지 결정할 수 있다)
 *
 * kevent는 ident와 filter를 식별자로 삼는다. 이를 통해 kevent의 중복 등록을 막고 해당 이벤트 발생조건이 부합하지 않을 경우(파일 디스크립터가 close될 경우 등)에는 kqueue에 의해 삭제되어 최소한의 kevent만 남을 수 있도록 관리한다.
 *
 * kqueue()로 kqueue를 할당하고, 이벤트의 변화(등록, 삭제 등)를 kevent()에 전달, kevent()가 반환한 처리가능한 event를 받아 그에 맞게 처리한다.
 *
 * kqueue는 select나 poll에 비해 이벤트 처리에서 효율적인 이유
 * 이벤트 발생 시, 해당 이벤트에 접근하는 시간복잡도가 O(1)이다.
 * select와 poll의 경우 이벤트 발생 시 해당 이벤트에 접근하는 시간복잡도가 O(N)이다. 등록된 파일 디스크립터를 하나하나 체크해서 커널과 유저 공간 사이에 여러번의 데이터 복사를 진행때문
 * kqueue는 발생한 이벤트를 정리하여 return해주기 때문에 O(1)로 접근 가능하며 등록된 이벤트를 따로 관리할 필요가 없다.
 * select는 fd_set, poll은 poll_fd 구조체의 배열로 모니터링할 이벤트들을 사용자가 관리하고, 이를 select()나 poll()에 전달해야 하지만, kqueue의 경우 새로 등록할 이벤트, 발생한 이벤트만 관리해주면 된다. 모니터링되는 이벤트는 kqueue, 즉 커널에서 관리된다.
 */

IRCEvent::IRCEvent()
{}

IRCEvent::~IRCEvent()
{}

/**
 * @brief kevent struct initialize & setting
 */
void IRCEvent::event_set(uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void *udata)
{
	struct kevent curr_kevent;
	////EV_SET(&kev, ident, filter, flags, fflags, data, udata) 매크로 함수를 이용하여 kevent 구조체를 초기화 할 수 있다.
	EV_SET(&curr_kevent, ident, filter, flags, fflags, data, udata);
	/**
	 * @brief kevent function
	 *
	 * int kevent(int kq, const struct kevent *changelist, int nchanges, struct kevent *eventlist, int nevents, const struct timespec *timeout);
	 * kq = _kqueue;
	 * changelist = curr_kevent;
	 * nchanges = 1;
	 * eventlist = NULL;
	 * nevent = 0;
	 * timeout = NULL;
	 */

	::kevent(_kqueue, &curr_kevent, 1, NULL, 0, NULL);
}

int IRCEvent::kevent()
{
	/**
	 * @brief kevent
	 *
	 */
	_count = ::kevent(_kqueue, NULL, 0, _events, EVENTS_MAX, NULL);
	log::print() << _count << " new kevent" << log::endl;
	return (_count);
}

/**
 * @brief Event init
 *
 * 1. kqueue()를 통해 새로운 event queue를 만들고 반환한 fd를 _kqueue에 저장.
 * 2. 반환된 fd(_kqueue)를 새로 모니터링할 이벤트로 등록.
 *
 * @param socket_fd
 */
void IRCEvent::init(int socket_fd)
{
	if ((_kqueue = ::kqueue()) == -1)
	{
		log::print() << "kqueue failed errno: " << errno << ":" <<strerror(errno) << log::endl;
		exit(FAILURE);
	}
	log::print() << "kqueue: " << _kqueue << log::endl;
	event_set(socket_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
}

void IRCEvent::add(IRCClient* client)
{
	event_set(client->get_fd(), EVFILT_READ, EV_ADD, 0, 0, client);
	event_set(client->get_fd(), EVFILT_WRITE, EV_ADD | EV_DISABLE, 0, 0, client);
}

void IRCEvent::remove(int fd)
{
	event_set(fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
	event_set(fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
}

void IRCEvent::toggle(int EVFILT_TYPE)
{
	event_set(_events[_index].ident, EVFILT_TYPE, EV_DISABLE, 0, 0, (IRCClient*)_events[_index].udata);
	event_set(_events[_index].ident, (EVFILT_TYPE == EVFILT_READ ? EVFILT_WRITE : EVFILT_READ), EV_ENABLE, 0, 0, (IRCClient*)_events[_index].udata);
}

void IRCEvent::toggle(IRCClient& client, int EVFILT_TYPE)
{
	event_set(client.get_fd(), EVFILT_TYPE, EV_DISABLE, 0, 0, &client);
	event_set(client.get_fd(), (EVFILT_TYPE == EVFILT_READ ? EVFILT_WRITE : EVFILT_READ), EV_ENABLE, 0, 0, &client);
}

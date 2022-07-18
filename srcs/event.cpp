#include "event.hpp"

Event::Event()
{}

Event::~Event()
{}

//EV_SET(&kev, ident, filter, flags, fflags, data, udata);
void Event::event_set(uintptr_t ident, int16_t filter,
	uint16_t flags, uint32_t fflags, intptr_t data, void *udata)
{
	struct kevent curr_kevent;
	EV_SET(&curr_kevent, ident, filter, flags, fflags, data, udata);
	//int kevent(int kq, const struct kevent *changelist, int nchanges, struct kevent *eventlist, int nevents, const struct timespec *timeout);
	::kevent(_kqueue, &curr_kevent, 1, NULL, 0, NULL);
}

void Event::toggle(int EVFILT_TYPE)
{
	event_set(_events[_index].ident, EVFILT_TYPE, EV_DISABLE, 0, 0, (Client*)_events[_index].udata);
	event_set(_events[_index].ident, EVFILT_TYPE == EVFILT_READ ? EVFILT_WRITE : EVFILT_READ, EV_ENABLE, 0, 0, (Client*)_events[_index].udata);
}

void Event::toggle(Client& client, int EVFILT_TYPE)
{
	event_set(client.get_fd(), EVFILT_TYPE, EV_DISABLE, 0, 0, &client);
	event_set(client.get_fd(), EVFILT_TYPE == EVFILT_READ ? EVFILT_WRITE : EVFILT_READ, EV_ENABLE, 0, 0, &client);
}

void Event::init(int socket_fd)
{
	if ((_kqueue = ::kqueue()) == -1)
	{
		log::print() << "kqueue failed errno: " << errno << ":" <<strerror(errno) << log::endl;
		exit(FAILURE);
	}
	log::print() << "kqueue: " << _kqueue << log::endl;
	event_set(socket_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
}

int Event::kevent()
{
	_count = ::kevent(_kqueue, NULL, 0, _events, EVENTS_MAX, NULL);
	log::print() << _count << " new kevent" << log::endl;
	return (_count);
}

void Event::add(Client* client)
{
	event_set(client->get_fd(), EVFILT_READ, EV_ADD, 0, 0, client);
	event_set(client->get_fd(), EVFILT_WRITE, EV_ADD | EV_DISABLE, 0, 0, client);
}

void Event::remove(int fd)
{
	event_set(fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
	event_set(fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
}

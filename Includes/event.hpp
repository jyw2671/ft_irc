#ifndef EVENT_HPP
#define EVENT_HPP

#include "client.hpp"
#include "log.hpp"
#include <sys/event.h>
#include <sys/types.h>

class Client;

class Event
{
	private:
		Event(const Event&);
		Event& operator=(const Event&);

		//EV_SET(&kev, ident, filter, flags, fflags, data, udata);
		void event_set(uintptr_t ident, int16_t filter,
		uint16_t flags, uint32_t fflags, intptr_t data, void *udata);
		int _kqueue;
		int _count;

	protected:
		struct kevent _events[EVENTS_MAX];
		int		_index;

		Event();
		~Event();

		void	toggle(int EVFILT_TYPE);
		void	toggle(Client& client, int EVFILT_TYPE);
		void	init(int socket_fd);
		void	add(Client* client);
		void	remove(int fd);
		int		kevent();
};

#endif

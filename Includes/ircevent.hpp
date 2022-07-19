#ifndef IRCEVENT_HPP
#define IRCEVENT_HPP

#include "ircclient.hpp"
#include "irclog.hpp"
#include <sys/event.h>
#include <sys/types.h>

class IRCClient;

class IRCEvent
{
	private:
		IRCEvent(const IRCEvent&);
		IRCEvent& operator=(const IRCEvent&);

		//EV_SET(&kev, ident, filter, flags, fflags, data, udata);
		void event_set(uintptr_t ident, int16_t filter,
		uint16_t flags, uint32_t fflags, intptr_t data, void *udata);
		int _kqueue;
		int _count;

	protected:
		struct kevent _events[EVENTS_MAX];
		int		_index;

		IRCEvent();
		~IRCEvent();

		int		kevent();
		void	init(int socket_fd);
		void	add(IRCClient* client);
		void	remove(int fd);
		void	toggle(int EVFILT_TYPE);
		void	toggle(IRCClient& client, int EVFILT_TYPE);
};

#endif

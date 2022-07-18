#ifndef FT_IRC_HPP
#define FT_IRC_HPP

#include "channel.hpp"
#include "client.hpp"
#include "event.hpp"
#include "ircd.hpp"
#include "utils.hpp"
#include "socket.hpp"

class FT_IRC : public Socket, public Event, public IRCD
{
	public:
		friend class IRCD;

		FT_IRC(int port, const std::string password);
		~FT_IRC();
		void start();
	private:
		FT_IRC();
		FT_IRC(const FT_IRC&);
		FT_IRC& operator=(const FT_IRC&);

		void irc_accept();
		void irc_receive();
		void irc_send();
		void irc_command_handler();
		void irc_disconnect(std::string reason = "");
};
#endif

#ifndef IRCSERVER_HPP
#define IRCSERVER_HPP

#include "ircchannel.hpp"
#include "ircclient.hpp"
#include "ircevent.hpp"
#include "irccommand.hpp"
#include "ircutils.hpp"
#include "ircsocket.hpp"

class IRCServer : public IRCSocket, public IRCEvent, public IRCCommand
{
	public:
		friend class IRCCommand;

		IRCServer(int port, const std::string password);
		~IRCServer();
		void start();
	private:
		IRCServer();
		IRCServer(const IRCServer&);
		IRCServer& operator=(const IRCServer&);

		void irc_accept();
		void irc_receive();
		void irc_send();
		void irc_command_handler();
		void irc_disconnect(std::string reason = "");
		void irc_disconnected(std::string reason = "");
};
#endif

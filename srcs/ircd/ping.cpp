#include "../../includes/irccommand.hpp"

void
	IRCCommand::pong()
{
}

void
	IRCCommand::ping()
{
	m_to_client(cmd_pong_reply());
}

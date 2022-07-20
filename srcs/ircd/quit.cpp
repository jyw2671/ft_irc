#include "../../includes/irccommand.hpp"

void
    IRCCommand::quit()
{
    std::string message = "Quit";
    if (_request->parameter.size())
        message += " :" + _request->parameter[0];
    m_disconnect(message);
}

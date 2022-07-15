#include "ft_irc.hpp"
#include "ircserver.hpp"
#include "irc_exception.hpp"
#include "ircsession.hpp"
#include "ircstring.hpp"
#include "ircmessage.hpp"
#include "ircnumericmessage.hpp"
#include "ircbot.hpp"
#include <sstream>
#include <string>
#include <stdexcept>
#include <set>

IRCServer::IRCServer(const std::string& password)
    : _password(password), _clients(), _channels() {}

IRCServer::~IRCServer() {}

IRCSession* IRCServer::FindbyNick(const std::string& nick) const
{
    ClientMap::const_iterator it = _clients.find(nick);
    if (it == _client.end())
        return (NULL);
    return (it->second);
}

IRCChannel* IRCServer::FindChannel(cosnt std::string& channel)
{
    ChannelMap::const_iterator it = _channels.find(channel)
    if (it == _channels.end())
        return (NULL);
    return (it->second.Load());
}

void    IRCServer::onNickname(IRCSessionn& session, IRCMessage& msg)
{
    //메세지에 파라미터가 없다면 에러
    if (msg.SizeParam() == 0)
        throw irc_exception(ERR_NONIXKNAMEGIVEN, "NICK", "No nickname given");
}
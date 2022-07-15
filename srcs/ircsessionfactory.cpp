#include "ft_irc.hpp"
#include "ircsessionfactory.hpp"
#include "ircserver.hpp"
#include "ircsession.hpp"

IRCSessionFactory::IRCSessionFactory() {}
IRCSessionFactory::~IRCSessionFactory() {}
IRCSessionFactory::IRCSessionFactory(const IRCSessionFactory&) {}
IRCSessionFactory& IRCSessionFactory::operator=(const IRCSessionFactory&) { return *this; }

IRCSessionFactory::IRCSessionFactory(IRCServer* server)
    : _server(server) {}

Session*
IRCSessionFactory::CreateSession(Channel* channel, int socketfd, int socketId, const std::string& addr)
{
    return new IRCSession(_server, channel, socketfd, socketId, addr);
}

#include "../includes/ircclient.hpp"
#include "../includes/ircchannel.hpp"
#include "../includes/ircserver.hpp"

IRCClient::IRCClient(sockaddr_in addr, int fd) : _addr(addr), _fd(fd)
{
	_buffers.requests.from = this;
	_names.host = inet_ntoa(_addr.sin_addr);
	_names.server = NAME_SERVER;
	_status.regsistered = 0;
	_buffers.offset = 0;
	_buffers.to_client.offset = 0;
}

IRCClient::IRCClient()
{
}

IRCClient::~IRCClient()
{
}

//getter
sockaddr_in	IRCClient::get_addr()
{
	return (_addr);
}

int	IRCClient::get_fd()
{
	return (_fd);
}

char*	IRCClient::get_IP()
{
	return (inet_ntoa(_addr.sin_addr));
}

const IRCClient::t_names&	IRCClient::get_names() const
{
	return (_names);
}

IRCClient::t_buffers&	IRCClient::get_buffers()
{
	return (_buffers);
}

const std::set<IRCChannel*>&	IRCClient::get_channels() const
{
	return (_channels);
}

std::string	IRCClient::get_nickmask()
{
	return ((_names.nick.empty() ? "*" : _names.nick) + "!"
			+ (_names.user.empty() ? "*" : _names.user) + "@"
			+ (_names.host.empty() ? "*" : _names.host));
}

void IRCClient::set_nickname(const std::string& nickname)
{
	_names.nick = nickname;
	_status.nick = 1;
}

void IRCClient::set_username(const std::string& username)
{
	_names.user = username;
	_status.user = 1;
}

void IRCClient::set_realname(const std::string& realname)
{
	_names.real = realname;
}

void IRCClient::set_status(e_type type)
{
	switch (type)
	{
		case PASS:
			_status.pass = 1;
			break;
		case NICK:
			_status.nick = 1;
			break;
		case USER:
			_status.user = 1;
			break;
		default:
			break;
	}
}

bool IRCClient::is_registered() const
{
	return (_status.regsistered == REGISTERED);
}

bool IRCClient::is_joined(IRCChannel* channel)
{
	return (_channels.count(channel));
}

void IRCClient::joined(IRCChannel *channel)
{
	_channels.insert(channel);
}

void IRCClient::parted(IRCChannel* channel)
{
	_channels.erase(channel);
}

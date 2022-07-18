#include "../includes/client.hpp"

Client::Client(sockaddr_in addr, int fd) : _addr(addr), _fd(fd)
{
	_buffers.requests.from = this;
	_names.host = inet_ntoa(_addr.sin_addr);
	_names.server = NAME_SERVER;
	_status.regsistered = 0;
	_buffers.offset = 0;
	_buffers.to_client.offset = 0;
}

Client::Client()
{
}

Client::~Client()
{
}

//getter

sockaddr_in	Client::get_addr()
{
	return (_addr);
}

int	Client::get_fd()
{
	return (_fd);
}

char*	Client::get_IP()
{
	return (inet_ntoa(_addr.sin_addr));
}

const Client::t_names&	Client::get_names() const
{
	return (_names);
}

Client::t_buffers&	Client::get_buffers()
{
	return (_buffers);
}

const std::set<Channel*>&	Client::get_channels() const
{
	return (_channels);
}

std::string	Client::get_nickmask()
{
	return ((_names.nick.empty() ? "*" : _names.nick) + "!"
			+ (_names.user.empty() ? "*" : _names.user) + "@"
			+ (_names.host.empty() ? "*" : _names.host));
}

void Client::set_nickname(const std::string& nickname)
{
	_names.nick = nickname;
	_status.nick = 1;
}

void Client::set_username(const std::string& username)
{
	_names.user = username;
	_status.user = 1;
}

void Client::set_realname(const std::string& realname)
{
	_names.real = realname;
}

void Client::set_status(e_type type)
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

bool Client::is_registered() const
{
	return (_status.regsistered == REGISTERED);
}

bool Client::is_joined(Channel* channel)
{
	return (_channels.count(channel));
}

void Client::joined(Channel *channel)
{
	_channels.insert(channel);
}

void Client::parted(Channel* channel)
{
	_channels.erase(channel);
}

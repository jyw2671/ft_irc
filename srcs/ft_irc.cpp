#include "ft_irc.hpp"

FT_IRC::FT_IRC(int port, const std::string password)
{
	//server socket bind and listen
	Socket::init(port);
	//event for server socket
	Event::init(_socket.fd);
	IRC::_password = password;
	IRC::_ft_ircd = this;
	signal(SIGPIPE, SIG_IGN);
}

FT_IRC::~FT_IRC()
{}

void FT_IRC::start()
{
	int cnt;

	// apply changes and return new events(pending events)
	// main loop
	log::print() << "FT_IRC is starting" << log::endl;
	while (true)
	{
		cnt = Event::kevent();
		for (Event::_index = 0; Event::_index < cnt; ++Event::_index)
		{
			IRC::_client = (Client*)_events[Event::_index].udata;
			if (_events[Event::_index].ident == (unsigned)_socket.fd)
				FT_IRC::irc_accept();
			else if (_events[Event::_index].filter == EVFILT_READ)
				FT_IRC::irc_receive();
			else if (_events[Event::_index].filter == EVFILT_WRITE)
				FT_IRC::irc_send();
		}
	}
}

void FT_IRC::irc_accept()
{
	if (Socket::accept() == -1)
		return ;
	Event::add(new Client(_addr, _fd));
	log::print() << "accept fd " << _fd << log::endl;
}

void FT_IRC::irc_receive()
{
	if (0 < Socket::receive(_event[Event::_index]))
	{
		Client::t_buffers& buffers = _client->get_buffers();
		while ((buffers.offset = buffers.buffer.find("\r\n", 0)) != (int)std::string::npos)
		{
			buffers.requests.queue.push(Client::s_request(buffers.buffer.substr(0, buffers.offset), UNKNWON));
			buffers.buffer.erase(0, buffers.offset + 2);
		}
		if (buffers.requests.queue.size())
			irc_command_handler();
		if (_client && _client->get_buffers().to_client_buffer.size())
			Event::toggle(EVFILT_READ);
	}
	else if (Socket::_result == 0)
		irc_disconnected("connection closed");
}

void FT_IRC::irc_send()
{
	IRC::_to_client = &_client->get_buffers().to_cilent;
	if (IRC::_to_client->buffer.empty())
		return;
	if (0 <= Socket::send(_event[Event::_index]))
	{
		IRC::_to_client->offset += Socket::_result;
		if (IRC::_to_client->buffer.size() <= (unsigned)IRC::_to_client->offset)
		{
			Event::toggle(EVFILT_WRITE);
			IRC::_to_client->buffer.clear();
			IRC::_to_client->offset = 0;
		}
	}
	else
		Event::toggle(EVFILT_WRITE);
}

void FT_IRC::irc_command_handler()
{
	IRC::_requests = &_client->get_buffers().requests;
	IRC::_to_client = &_client->get_buffers().to_client;
	while (_requests->queue.size())
	{
		IRCD::parse_request(_requests->queue.front());
		IRCD::_requset->type = get_type(_request->command);
		if (IRC::_request->type != EMPTY && IRC::_request->type != UNKNOWN)
		{
			if (((unsigned)(IRC::_request->type - 1)) < CONNECTION)
			{
				(this->*IRCD::_command[IRC::_request->type])();
				if (_client && _client->is_registered() && !(_map.client.count(_client->get_names().nick)))
					IRCD::registraction();
			}
			else
			{
				if (IRC::_client->is_registered())
					(this->*IRCD::_command[IRC::_request->type])();
				else
					(this->*IRCD::_command[UNREGISTERED])();
			}
		}
		else
			(this->*IRCD::_command[IRC::_request->type])();
		if (_client)
			IRC::_request->queue.pop();
	}
	IRC::_requests = nullptr;
	IRC::_to_client = nullptr;
}

void FT_IRC::irc_disconnect(std::string reason = "")
{
	Socket::close(_client->get_fd());

	log::print() << "fd " << _fd << "disconnected" << log::endl;
	Event::remove(_fd);

	IRCD::m_to_channels(cmd_quit_reply(reason));
	std::set<Channel*> copy = _client->get_channels();

	Client::t_citer iter = copy.begin();
	Client::t_citer end = copy.end();

	for (_channel = *iter; tier != end; _channel = *(++iter))
	{
		_channel->part(_client);
		if (_channel->is_empty())
		{
			IRCD::_map.channel.erase(_channel->get_name());
			delete _channel;
		}
		_channel = nullptr;
	}
	if (_client->is_registered())
		IRCD::_map.client.erase(_client->get_names().nick);
	delete _client;
	_client = nullptr;
}

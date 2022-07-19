#include "ircserver.hpp"

//http://forum.falinux.com/zbxe/index.php?document_srl=406054&mid=network_programming

/**
 * @brief Construct a new IRCServer::IRCServer object
 *
 * @param port 연결할 포트 번호입니다.
 * @param password 설정할 비밀번호입니다.
 */
IRCServer::IRCServer(int port, const std::string password)
{
	//socket for server connect
	IRCSocket::init(port);
	//event for server socket
	IRCEvent::init(_socket.fd);
	//irc message set
	IRCMessage::_password = password;
	IRCMessage::_ircserver = this;
	// signal handler
	//https://jacking75.github.io/linux_socket_sigpipe/
	signal(SIGPIPE, SIG_IGN);
}

IRCServer::~IRCServer()
{}

/**
 * @brief IRCServer start(run)
 *
 * apply changes and return new events(pending events)
 * main loop
 */
void IRCServer::start()
{
	//아직 처리되지 않은(pending 상태) 이벤트의 개수를 저장하기 위한 변수
	int count;

	log::print() << "IRCServer is starting" << log::endl;
	while (true)
	{
		count = IRCEvent::kevent();
		/**
		 * @brief loop
		 *
		 */
		for (IRCEvent::_index = 0; IRCEvent::_index < count; ++IRCEvent::_index)
		{
			IRCMessage::_client = (IRCClient*)_events[IRCEvent::_index].udata;
			//accept new client
			if (_events[IRCEvent::_index].ident == (unsigned)_socket.fd)
				IRCServer::irc_accept();
			//read data from client
			else if (_events[IRCEvent::_index].filter == EVFILT_READ)
				IRCServer::irc_receive();
			//send data to client
			else if (_events[IRCEvent::_index].filter == EVFILT_WRITE)
				IRCServer::irc_send();
		}
	}
}

void IRCServer::irc_accept()
{
	if (IRCSocket::accept() == -1)
		return ;
	IRCEvent::add(new IRCClient(_addr, _fd));
	log::print() << "accept fd " << _fd << log::endl;
}

void IRCServer::irc_receive()
{
	if (0 < IRCSocket::receive(_events[IRCEvent::_index]))
	{
		IRCClient::t_buffers& buffers = _client->get_buffers();
		while ((buffers.offset = buffers.buffer.find("\r\n", 0)) != (int)std::string::npos)
		{
			buffers.requests.queue.push(IRCClient::s_request(buffers.buffer.substr(0, buffers.offset), UNKNOWN));
			buffers.buffer.erase(0, buffers.offset + 2);
		}
		if (buffers.requests.queue.size())
			irc_command_handler();
		//client receive
		if (_client && _client->get_buffers().to_client.buffer.size())
			IRCEvent::toggle(EVFILT_READ);
	}
	else if (IRCSocket::_result == 0)
		irc_disconnected("connection colsed");
}

void IRCServer::irc_send()
{
	IRCMessage::_to_client = &_client->get_buffers().to_client;
	if (IRCMessage::_to_client->buffer.empty())
		return;
	if (0 <= IRCSocket::send(_events[IRCEvent::_index]))
	{
		IRCMessage::_to_client->offset += IRCSocket::_result;
		if (IRCMessage::_to_client->buffer.size() <= (unsigned)IRCMessage::_to_client->offset)
		{
			IRCEvent::toggle(EVFILT_WRITE);
			IRCMessage::_to_client->buffer.clear();
			IRCMessage::_to_client->offset = 0;
		}
	}
	else
		IRCEvent::toggle(EVFILT_WRITE);
}

void IRCServer::irc_command_handler()
{
	IRCMessage::_requests = &_client->get_buffers().requests;
	IRCMessage::_to_client = &_client->get_buffers().to_client;
	while (_requests->queue.size())
	{
		IRCCommand::parse_request(_requests->queue.front());
		IRCCommand::_request->type = get_type(_request->command);
		if (IRCMessage::_request->type != EMPTY && IRCMessage::_request->type != UNKNOWN)
		{
			if (((unsigned)(IRCMessage::_request->type - 1)) < CONNECTION)
			{
				(this->*IRCCommand::_commands[IRCMessage::_request->type])();
				if (_client && _client->is_registered() && !(_map.client.count(_client->get_names().nick)))
					IRCCommand::registration();
			}
			else
			{
				if (IRCMessage::_client->is_registered())
					(this->*IRCCommand::_commands[IRCMessage::_request->type])();
				else
					(this->*IRCCommand::_commands[UNREGISTERED])();
			}
		}
		else
			(this->*IRCCommand::_commands[IRCMessage::_request->type])();
		if (_client)
			IRCMessage::_requests->queue.pop();
	}
	IRCMessage::_requests = nullptr;
	IRCMessage::_to_client = nullptr;
}

void IRCServer::irc_disconnected(std::string reason = "")
{
	//disconnected
	log::print() << "fd " << _fd << "disconnected" << log::endl;
	IRCEvent::remove(_fd);
	IRCCommand::m_to_channels(cmd_quit_reply("connection closed"));

	std::set<IRCChannel*> copy = _client->get_channels();

	IRCClient::t_citer iter = copy.begin();
	IRCClient::t_citer end = copy.end();
	for (_channel = *iter; iter != end; _channel = *(++iter))
	{
		_channel->part(_client);
		if (_channel->is_empty())
		{
			IRCCommand::_map.channel.erase(_channel->get_name());
			delete _channel;
		}
		_channel = nullptr;
	}
	if (_client->is_registered())
		IRCCommand::_map.client.erase(_client->get_names().nick);
	delete _client;
	_client = nullptr;
}

void IRCServer::irc_disconnect(std::string reason = "")
{
	IRCSocket::close(_client->get_fd());
	irc_disconnected(reason);
}

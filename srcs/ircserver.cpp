#include "../includes/ircserver.hpp"

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

/**
 * @brief server_accept()
 *
 * socket에서 accept() 함수를 통해 반환된 client socket discriptor를 사용하여 연결할 클라이언트를 생성
 * 생성된 클라이언트의 fd를 통해 클라이언트와 연결을 기다린다.
 */
void IRCServer::irc_accept()
{
	if (IRCSocket::accept() == -1)
		return ;
	IRCEvent::add(new IRCClient(_addr, _fd));
	log::print() << "accept fd " << _fd << log::endl;
}

/**
 * @brief server_receive()
 *
 * kevent 구조체 배열에 존재하는 이벤트를 모니터링하여 클라이언트로 부터 받을 준비를 한다.
 *
 */
void IRCServer::irc_receive()
{
	/**
	 * @brief if문 동작
	 *
	 * IRCSocket::receive 함수의 반환값이 0 이상이란 의미는 정상적으로 데이터를 수신하였다는 의미이며, 반환값은 실제로 수신한 데이터의 길이이다.
	 * IRCSocket::receive 함수에서 _fd와 _result 변수 값 저장
	 */
	if (0 < IRCSocket::receive(_events[IRCEvent::_index]))
	{
		/**
		 * @brief buffers
		 *
		 * 1. client에서 보낸 메시지를 저장할 버퍼 : buffers 구조체 변수를 설정
		 * 2. buffers 구조체 내부의 std::string buffer에 client에서 보낸 메시지 저장
		 * 3. _buffer, _result는 IRCSocket::receive에서 recv함수를 통해 설정
		 * 4. std::string buffer에 "\r\n"을 찾고 위치를 반환하여 IRCClient::s_request 구조체에 command와 type을 넣는다.
		 * 5. client에 들어온 명령어를 저장한 구조체를 buffers.requests의 queue에 저장한다. -> 저장한 queue에는 명령어와 type이 담겨있다.
		 */
		IRCClient::t_buffers& buffers = _client->get_buffers();
		/**
		 * @brief append()
		 *
		 * IRCSocket::_buffer type : char
		 * IRCSocket::_result type : ssize_t
		 * _buffer 부터 _buffer + _result 전 까지의 문자들을 문자열 뒤에 붙인다. 이 때 해당 구간안에 NULL 문자가 있어도 괜찮다.
		 */
		buffers.buffer.append(IRCSocket::_buffer, IRCSocket::_result);
		/**
		 * @brief find & substr
		 *
		 * find()
		 * buffer 문자열에서 "\r\n"의 위치를 찾아 반환한다.
		 * 문자열을 찾았다면 그 위치를 반환하고, 찾고자하는 문자열이 없을 경우 npos를 반환한다.
		 *
		 * substr()
		 * basic_string substr(size_type pos = 0, size_type count = npos) const;
		 * 문자열의 pos 번째 문자 부터 count 길이 만큼의 문자열을 리턴한다.
		 *
		 * erase()
		 * basic_string& erase(size_type index = 0, size_type count = npos);
		 * index 로 부터, count 개의 문자들을 지운다. (substr 랑 비슷하다고 생각하면 된다.)
		 * 만일 count가 문자열 끝을 넘어간다면, 그 이상 지우지 않는다.
		 * string 에서 원하는 범위의 문자들을 삭제한다.
		 */
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

/**
 * @brief irc_send()
 *
 * client로 메시지를 전달한다.
 */
void IRCServer::irc_send()
{
	IRCMessage::_to_client = &_client->get_buffers().to_client;
	/**
	 * @brief send
	 *
	 * 보낼 메시지가 없으면 return;
	 *
	 */
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

/**
 * @brief irc_command_handler()
 *
 * client에서 입력받은 메시지의 명령어를 처리한다.
 *
 */
void IRCServer::irc_command_handler()
{
	/**
	 * @brief IRCMessage
	 *
	 * IRCMessage::_requests : client 버퍼에 저장된 요청 메시지 사항
	 * IRCMessage::t_to_client* : 메시지 처리를 위해 사용
	 */
	IRCMessage::_requests = &_client->get_buffers().requests;
	IRCMessage::_to_client = &_client->get_buffers().to_client;
	while (_requests->queue.size())
	{
		/**
		 * @brief
		 * _request에 들어있는 type을 파악하여 비어있거나 알 수 없는 명령어인지 확인
		 * parse_request에서 size를 확인하고 :를 통해 명령어가 들어오는 경우도 있으니 확인한다.
		 */
		IRCCommand::parse_request(_requests->queue.front());
		IRCCommand::_request->type = get_type(_request->command);
		if (IRCMessage::_request->type != EMPTY && IRCMessage::_request->type != UNKNOWN)
		{
			/**
			 * @brief
			 * _commands : typedef std::vector<void (IRCCommand::*)()>의 벡터
			 * _map : client와 channel의 정보를 저장하고 있는 map
			 * _client의 t_status 상태를 확인하는 구조체에 등록된 pass, nick, user인지 확인
			 * _client의 t_names 구조체 안의 nick 개수가 0이 아니라면 설정한 nick을 등록한다.
			 *
			 * type < CONNECTION(4) == PASS, NICK, USER
			 * type = 4 == QUIT
			 */
			if (((unsigned)(IRCMessage::_request->type - 1)) < CONNECTION)
			{
				(this->*IRCCommand::_commands[IRCMessage::_request->type])();
				if (_client && _client->is_registered() && !(_map.client.count(_client->get_names().nick)))
					IRCCommand::registration();
			}
			/**
			 * @brief
			 *
			 * 	type > CONNECTION(4) == PART, JOIN ~
			 */
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
/**
 * @brief irc_disconnected()
 *
 * client에서 수신할 메시지가 없으면 연결을 종료한다.
 *
 * @param reason
 */
void IRCServer::irc_disconnected(std::string reason)
{
	//disconnected
	log::print() << " fd " << _fd << " disconnected" << log::endl;
	IRCEvent::remove(_fd);
	IRCCommand::m_to_channels(cmd_quit_reply(reason));

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

/**
 * @brief irc_disconnect
 *
 * quit 명령어가 들어왔을 경우 client fd close 후 연결을 해제한다.
 *
 * @param reason
 */
void IRCServer::irc_disconnect(std::string reason)
{
	IRCSocket::close(_client->get_fd());
	irc_disconnected(reason);
}

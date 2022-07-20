#include "../../includes/irccommand.hpp"

/**
 * @brief server에 client 등록하는 절차
 *
 * - 등록이 완료될 때까지는 PASS, NICK, USER만 사용가능
 * - 등록이 완료되어야 irc를 이용할 수 있음.
 */


/**
 * @brief pass
 *
 * - server에 설정된 비밀번호를 입력하는 명령어
 * - ./ircserv [port] [pw]에서 pw가 PASS의 파라미터로 와야함.
 * - 비밀번호가 맞지 않으면 pass 명령을 무시하고 server가 에러로 응답함.
 * - 명령어 형식
 * 		PASS [password]
 */

e_result IRCCommand::m_pass()
{
	if (_request->parameter.empty())
		return (m_to_client(err_need_more_params()));
	if (_client->is_registered())
		return (m_to_client(err_already_registred()));
	if (_request->parameter[0] != _password)
		return (m_to_client(err_passwd_mismatch()));
	return (OK);
}

void IRCCommand::pass()
{
	if (m_pass() == ERROR)
		return;
	_client->set_status(PASS);
}

/**
 * @brief nick
 *
 * - server에 nickname을 등록하거나 변경하는 명령어
 * - 다음의 상황에서 nick 명령을 무시하고 server는 에러를 응답함
 * 		-> nickname이 이미 사용중일 때
 * 		-> 등록할 nickname이 잘못된 형식일 때
 * 		-> nickname을 파라미터로 보내지 않았을 때
 * - 명령어 형식
 * 		NICK [nickname]
 */

e_result IRCCommand::m_nick()
{
	if (_request->parameter.empty())
		return (m_to_client(err_no_nickname_given()));
	_target = &_request->parameter[0];
	if (m_is_valid(NICK) == ERROR)
		return (m_to_client(err_erroneus_nickname(*_target)));
	if (_map.client.count(*_target))
	{
		if (*_target != _client->get_names().nick)
			return (m_to_client(err_nickname_in_use(*_target)));
		return (ERROR);
	}
	return (OK);
}

void IRCCommand::nick()
{
	if (m_nick() == ERROR)
		return;
	if (_client->is_registered())
	{
		if (_client->get_channels().size())
			m_to_channels(cmd_nick_reply(*_target));
		_map.client.erase(_client->get_names().nick);
		_map.client[*_target] = _client;
		m_to_client(cmd_nick_reply(*_target));
	}
	_client->set_nickname(*_target);
	log::print() << "fd " << _client->get_fd()
		<< " client nick: " << _client->get_names().nick << log::endl;
}

/**
 * @brief user
 *
 * - 유저 정보를 등록하는 명령어
 * - 유저 정보는 username, hostname, servername, realname이 있음.
 * - hostname, servername은 server 간 통신에 사용됨.
 * - realname은 :이 앞에 붙어있어서 공백이 추가될 수 있기 때문에 맨 마지막 파라미터로 와야 함.
 * - 다음과 같은 상황에서 user 명령을 무시하고 server가 에러를 응답함.
 * 		-> 파라미터가 4개보다 작게 들어올 때
 * 		-> client가 이미 server 등록을 완료했는데 USER 명령어를 사용하려고 할 때
 * - 명령어 형식
 * 		USER [username] [hostname] [servername] [realname]
 */

e_result IRCCommand::m_user()
{
	if (_request->parameter.size() < 4)
		return (m_to_client(err_need_more_params()));
	if (_client->is_registered())
		return (m_to_client(err_already_registred()));
	return (OK);
}

void IRCCommand::user()
{
	if (m_user() == ERROR)
		return;
	_client->set_username(_request->parameter[0]);
	_client->set_realname(_request->parameter[3]);
}

/**
 * @brief user
 *
 * - server에 대한 client 연결을 종료하는 명령어
 * - client가 보낸 QUIT 명령에 의해 연결이 종료되면
 * 		server는 QUIT 메시지를 다른 client에 보낼 때
 * 		[reason] 앞에 문자열 "Quit:"를 추가하여 이 사용자가 스스로 연결을 종료했음을 나타내야 함.
 * - [reason]이 생략되면 빈 상태로 보냄.
 * - 명령어 형식
 * 		QUIT [reason(생략가능)]
 */

void IRCCommand::quit()
{
	std::string message = "Quit";
	if (_request->parameter.size())
		message += " :" + _request->parameter[0];
	m_disconnect(message);
}

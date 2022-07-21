#include "../../includes/irccommand.hpp"

/**
 * @brief privmsg
 *
 * - 채널이나 클라이언트에게 보내는 메시지
 * - PRIVMSG 는 유저 끼리 귓속말을 하는 역할을 한다.
 * - 채널에서 하는 채팅은 PRIVMSG를 채널로 보내는 방식으로 동작한다.
 * - 명령어 형식
 * 		privmsg user :message
 * 		:user1 PRIVMSG user2 :message
 */

e_result IRCCommand::m_privmsg(e_phase phase)
{
	if (phase == ONE)
	{
		if (_request->parameter.empty())
			return (m_to_client(err_no_recipient()));
		if (_request->parameter.size() == 1)
			return (m_to_client(err_no_text_to_send()));
	}
	if (phase == TWO)
	{
		if (!_map.channel.count(*_target))
			return (m_to_client(err_no_such_channel(*_target)));
		_channel = _map.channel[*_target];
		if (_channel->get_status(NOMSG) && !_channel->is_joined(_client))
			return (m_to_client(err_cannot_send_to_channel(_channel->get_name(), 'n')));
	}
	return (OK);
}

void IRCCommand::privmsg()
{
	if (m_privmsg(ONE) == ERROR)
		return;
	t_cstr_vector targets = split(_request->parameter.front(), ',');
	for (int i = 0, size = targets.size(); i < size; ++i)
	{
		_target = &targets[i];
		if (m_is_valid(CHANNEL_PREFIX))
		{
			if (m_privmsg(TWO) == OK)
				m_to_channel(cmd_message_reply(*_target));
		}
		else if (_map.client.count(*_target))
			m_to_client(*_map.client[*_target], cmd_message_reply(*_target));
		else if (_request->type == PRIVMSG)
			m_to_client(err_no_such_nick(*_target));
	}
}

/**
 * @brief notice
 *
 * - 통지 (자동 회신을 원하지 않는 메시지)
 * - 명령어 형식
 *		notice #channel :message
 */

void IRCCommand::notice()
{
    privmsg();
}

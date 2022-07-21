#include "../../includes/irccommand.hpp"

/**
 * @brief 채널 명령어
 */

/**
 * @brief join
 *
 * - 클라이언트가 특정 채널에 가입할 때 사용함.
 * - 다음과 같은 상황일 때 join 명령을 무시하고 서버가 에러를 응답함.
 * 		파라미터 개수가 형식보다 적을 때
 * 		클라이언트가 참여할 수 있는 채널의 최대 수에 도달했을 때
 * 		키가 필요한 채널에 키가 잘못되었거나 제공되지 않았을 때
 * 		채널이 유효하지 않을 때
 * 		채널이 수용할 수 있는 클라이언트의 최대 수에 도달했을 때
 * 		채널이 invite-only 모드인 데 채널 관리자로부터 초대를 받지 않았을 때
 *
 * - 명령어 형식
 * 		join [해당 채널]
 */

e_result IRCCommand::m_join(e_phase phase, IRCChannel* channel)
{
	if (phase == ONE)
	{
		if (_request->parameter.empty())
			return (m_to_client(err_need_more_params()));
	}
	else if (phase == TWO)
	{
		if (!m_is_valid(CHANNEL_PREFIX))
			return (m_to_client(err_no_such_channel(*_target)));
		if (!m_is_valid(CHANNEL_NAME))
			return (m_to_client(err_no_such_channel(*_target)));
		if (CLIENT_CAHNNEL_MAX <= _client->get_channels().size())
			return (m_to_client(err_too_many_channels(*_target)));
	}
	else if (phase == THREE)
	{
		if (_client->is_joined(channel))
			return (ERROR);
		else if (channel->is_full())
			return (m_to_client(err_channel_is_full(*_target)));
		else if ((_channel->get_status(INVITE))
				&& (!_channel->is_invited(_client)))
			return (m_to_client(err_invite_only_channel(_channel->get_name())));
	}
	return (OK);
}

void IRCCommand::join()
{
	if (m_join(ONE) == ERROR)
		return;
	t_cstr_vector channels = split(_request->parameter[0], DELIMITER);
	IRCCommand::t_iter iter = channels.begin();
	IRCCommand::t_iter end = channels.end();
	for (; iter != end; ++iter)
	{
		_target = iter.base();
		if (m_join(TWO) == ERROR)
			continue;
		if (!_map.channel.count(*_target))
		{
			_map.channel.insert(
				std::make_pair(*_target, new IRCChannel(*_target, _client)));
			_channel = _map.channel[*_target];
		}
		else
		{
			_channel = _map.channel[*_target];
			if (m_join(THREE, _channel) == ERROR)
				continue;
			_channel->join(_client);
		}
		m_to_channel(cmd_join_reply(*_target));
		m_to_client(cmd_join_reply(*_target));
		if (_channel->get_topic().size())
			m_to_client(rpl_topic(_channel->get_name(), _channel->get_topic()));
		else
			m_to_client(rpl_notopic(_channel->get_name()));
		m_names();
	}

}

/**
 * @brief part
 *
 * - 클라이언트가 채널에서 나올 때 사용하는 명령어
 * - 명령어 형식
 * 		PART #A : 채널 #A에서 나가기
 * 		PART #B,&C : 채널 #B,&C에서 나가기
 */

e_result IRCCommand::m_part(e_phase phase)
{
	if (phase == ONE)
	{
		if (_request->parameter.empty())
			return (m_to_client(err_need_more_params()));
	}
	else if (phase == TWO)
	{
		if (!_map.channel.count(*_target))
			return (m_to_client(err_no_such_channel(*_target)));
		_channel = _map.channel[*_target];
		if (!_channel->is_joined(_client))
			return (m_to_client(err_not_on_channel(*_target)));
	}
	return (OK);
}

void IRCCommand::part()
{
	if (m_part(ONE) == ERROR)
		return;
	t_cstr_vector channels = split(_request->parameter[0], ',');
	for (int i = 0, size = channels.size(); i < size; ++i)
	{
		_target = &channels[i];
		if (m_part(TWO) == ERROR)
			return;
		_channel->part(_client);
		m_to_channel(cmd_part_reply(*_target));
		m_to_client(cmd_part_reply(*_target));
		if (_channel->is_empty())
		{
			_map.channel.erase(_channel->get_name());
			delete _channel;
		}
		log::print() << "remove " << _client->get_names().nick
					 << " client from " << _channel->get_name() << " channel"
					 << log::endl;
	}
}

/**
 * @brief names
 *
 * - 볼 수 있는 모든 채널에서 볼 수 있는 모든 닉네임을 출력.
 * - 명령어 형식
 * 		NAMES
 *		NAMES #ch1
 */

e_result IRCCommand::m_names()
{
	_channel = _map.channel[*_target];
	_buffer  = "= " + _channel->get_name() + " :";
	if (_channel->get_operator())
		_buffer.append("@" + _channel->get_operator()->get_names().nick + " ");
	IRCChannel::t_citer_member iter = _channel->get_members().begin();
	IRCChannel::t_citer_member end  = _channel->get_members().end();
	for (; iter != end; ++iter)
		_buffer.append((*iter)->get_names().nick + " ");
	m_to_client(rpl_namereply(_buffer));
	m_to_client(rpl_endofnames(_channel->get_name()));
	_buffer.clear();
	return (OK);
}

void IRCCommand::names()
{
	if (_request->parameter.empty())
	{
		IRCMessage::t_iter_ch ch_iter = _map.channel.begin();
		for (; ch_iter != _map.channel.end(); ++ch_iter)
		{
			_target = &ch_iter->first;
			m_names();
		}
		IRCMessage::t_iter_cl cl_iter = _map.client.begin();
		for (; cl_iter != _map.client.end(); ++cl_iter)
			if (cl_iter->second->get_channels().empty())
				_buffer.append(cl_iter->first + " ");
		if (_buffer.size())
			m_to_client(rpl_namereply("= * :" + _buffer));
		m_to_client(rpl_endofnames("*"));
		_buffer.clear();
		return;
	}
	else
	{
		t_cstr_vector channels = split(_request->parameter[0], ',');
		for (int i = 0, size = channels.size(); i < size; ++i)
		{
			_target = &channels[i];
			if (_map.channel.count(*_target))
				m_names();
			else
				m_to_client(rpl_endofnames(*_target));
		}
	}
}

/**
 * @brief list
 *
 * - 채널과 해당 주제를 나열.
 * 		채널 파라미터가 생략되면 개설된 채널 모두의 상태를 나열.
 * 		채널 파라미터가 있으면 해당 채널 상태만 나열.
 * - 명령어 형식
 *		list
 * 		list #channel
 */

e_result IRCCommand::m_list()
{
	int member_number = _channel->get_members().size()
		+ _channel->get_operator() == nullptr ? 0 : 1;
	m_to_client(rpl_list(_channel->get_name(),
						 std::to_string(member_number),
						 _channel->get_topic()));
	return (OK);
}

void IRCCommand::list()
{
	if (_request->parameter.empty())
	{
		IRCMessage::t_iter_ch iter = _map.channel.begin();
		for (_channel = iter->second; iter != _map.channel.end();
				_channel = (++iter)->second)
			m_list();
	}
	else if (_request->parameter.size() == 1)
	{
		t_cstr_vector channels = split(_request->parameter[0], ',');
		for (int i = 0, size = channels.size(); i < size; ++i)
			if (_map.channel.count(channels[i])
					&& (_channel = _map.channel[channels[i]]))
				m_list();
			else
				m_to_client(err_no_such_channel(channels[i]));
	}
	m_to_client(rpl_listend());
}

/**
 * @brief topic
 *
 * - 채널의 주제를 보여주거나 주제를 변경할 때 사용하는 명령어
 * - 명령어 형식
 *		topic #channel :topic~
 */

e_result IRCCommand::m_topic()
{
	if (_request->parameter.empty())
		return (m_to_client(err_need_more_params()));
	_target = &_request->parameter[0];
	if (!_map.channel.count(*_target))
		return (m_to_client(err_no_such_channel(*_target)));
	_channel = _map.channel[*_target];
	if (!_channel->is_joined(_client))
		return (m_to_client(err_not_on_channel(*_target)));
	if ((1 < _request->parameter.size()) && (_channel->get_status(TOPIC))
			&& (!_channel->is_operator(_client)))
		return (m_to_client(err_chanoprivs_needed(*_target)));
	return (OK);
}

void IRCCommand::topic()
{
	if (m_topic() == ERROR)
		return;
	if (_request->parameter.size() == 1)
	{
		if (_channel->get_topic().size())
			m_to_client(rpl_topic(*_target, _channel->get_topic()));
		else
			m_to_client(rpl_notopic(*_target));
	}
	else
	{
		_channel->set_topic(_request->parameter[1]);
		log::print() << *_target << " channel topic: " << _channel->get_topic()
					 << log::endl;
		m_to_client(cmd_topic_reply());
		m_to_channel(cmd_topic_reply());
	}
}

/**
 * @brief invite
 *
 * - user를 초대함
 * - 초대 전용 모드가 아닐 때
 * 		실제로 해당 유저가 join 요청을 해야 멤버가 됨.
 * 		따라서 초대 전용 모드가 아니면 invite 명령어는 의미없음.
 * - 에러의 경우
 * 		파라미터 개수가 2 미만일 때
 * 		클라이언트가 존재하지 않을 때
 * 		채널이 존재하지 않을 때
 * 		초대하려는 클라이언트가 채널에 없을 때
 * 		초대 대상이 이미 채널 구성원일 때
 * - 명령어 형식
 *		invite nick #channel
 * 		:user1 invite user2 #channel
 * 	 - 초대 전용인 채널에 유저를 초대하려면
 * 			초대를 보내는 클라이언트가 해당 채널의 채널 운영자로 인식되어야 함.
 */

e_result IRCCommand::m_invite()
{
	if (_request->parameter.size() < 2)
		return (m_to_client(err_need_more_params()));
	if (!_map.client.count(_request->parameter[0]))
		return (m_to_client(err_no_such_nick(_request->parameter[0])));
	_fixed = _map.client[_request->parameter[0]];
	if (!_map.channel.count(_request->parameter[1]))
		return (m_to_client(err_no_such_channel(_request->parameter[1])));
	_channel = _map.channel[_request->parameter[1]];
	if (!_client->is_joined(_channel))
		return (m_to_client(err_not_on_channel(_request->parameter[1])));
	if (_fixed->is_joined(_channel))
		return (m_to_client(err_user_on_channel(_request->parameter[0],
												_request->parameter[1])));
	if ((_channel->get_status(INVITE)) && !(_channel->is_operator(_client)))
		return (m_to_client(err_chanoprivs_needed(_channel->get_name())));
	return (OK);
}

void IRCCommand::invite()
{
	if (m_invite() == ERROR)
		return;
	_channel->invitation(_fixed);
	m_to_client(rpl_inviting(_request->parameter[0], _request->parameter[1]));
	m_to_client(*_fixed, cmd_invite_reply(_request->parameter[0],
											_request->parameter[1]));
}

/**
 * @brief kick
 *
 * - 채널 관리자가 채널에 있는 인원을 추방 시킬 때 사용하는 명령어
 * - 에러의 경우
 * 		파라미터의 개수가 2개 미만일 때
 * 		채널이름과 닉네임 개수가 1:n, n:1이 아닐 때
 * 		채널이름과 닉네임 개수가 같지 않을 때
 * 		채널이 유효하지 않을 때
 * 		채널 관리자가 아닐 때
 * 		클라이언트가 존재하지 않을 때
 * 		강퇴 대상이 채널에 이미 없을 때
 * - 명령어 형식
 *		kick #channel user
 */

e_result IRCCommand::m_kick(e_phase phase)
{
	if (phase == ONE)
	{
		if (_request->parameter.size() < 2)
			return (m_to_client(err_need_more_params()));
	}
	else if (phase == TWO)
	{
		if ((!m_is_valid(CHANNEL_PREFIX) || !_map.channel.count(*_target)))
			return (m_to_client(err_no_such_channel(*_target)));
		_channel = _map.channel[*_target];
		if (!_channel->is_operator(_client))
			return (m_to_client(err_chanoprivs_needed(*_target)));
		if (!_map.client.count(*_target_sub))
			return (m_to_client(err_no_such_nick(*_target_sub)));
		_fixed = _map.client[*_target_sub];
		if (!_channel->is_joined(_fixed))
			return (m_to_client(err_user_not_in_channel(*_target_sub, *_target)));
	}
	return (OK);
}

void IRCCommand::kick()
{
	if (m_kick(ONE) == ERROR)
		return;
	t_cstr_vector param_0 = split(_request->parameter[0], ',');
	t_cstr_vector param_1 = split(_request->parameter[1], ',');
	if ((!(param_0.size() == 1 || param_1.size() == 1)
			&& param_0.size() != param_1.size()))
	{
		m_to_client(err_need_more_params());
		return;
	}
	IRCCommand::t_iter names = param_0.begin();
	IRCCommand::t_iter nicks = param_1.begin();
	for (int i = 0, max = std::max(param_0.size(), param_1.size()); i < max;
			++i)
	{
		_target     = names.base();
		_target_sub = nicks.base();
		if (m_kick(TWO) == ERROR)
			goto next;
		_channel->part(_fixed);
		if (_channel->is_empty())
		{
			_map.channel.erase(*names);
			delete _channel;
			m_to_client(
				cmd_kick_reply(*names, *nicks, _client->get_names().nick));
		}
		else
		{
			m_to_channel(
			    cmd_kick_reply(*names, *nicks, _client->get_names().nick));
			m_to_client(*_fixed, cmd_kick_reply(*names, *nicks,
								_client->get_names().nick));
			m_to_client(
				cmd_kick_reply(*names, *nicks, _client->get_names().nick));
		}
	next:
		if (param_0.size() != 1)
			++names;
		if (param_1.size() != 1)
			++nicks;
	}
}

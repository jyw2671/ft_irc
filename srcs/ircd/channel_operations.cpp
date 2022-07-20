#include "../../includes/irccommand.hpp"

/**
 * @brief 채널 명령어
 *
 * - 위 등록 절차를 완료하면 서버로부터 환영 메시지(rpl_welcome)를 받음.
 */

/**
 * @brief join
 *
 * - 명령어 형식
 *
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
 * - 명령어 형식
 *
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
 * - 명령어 형식
 *
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
 * - 명령어 형식
 *
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
 * - 명령어 형식
 *
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
 * - 명령어 형식
 *
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
 * - 명령어 형식
 *
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

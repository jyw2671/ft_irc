#include "ircmessage.hpp"

IRCMessage::IRCMessage() : endl("\r\n")
{
	_command_to_type.insert(std::make_pair("", EMPTY));
	_command_to_type.insert(std::make_pair("PASS", PASS));
	_command_to_type.insert(std::make_pair("NICK", NICK));
	_command_to_type.insert(std::make_pair("USER", USER));
	_command_to_type.insert(std::make_pair("QUIT", QUIT));
	_command_to_type.insert(std::make_pair("JOIN", JOIN));
	_command_to_type.insert(std::make_pair("PART", PART));
	_command_to_type.insert(std::make_pair("TOPIC", TOPIC));
	_command_to_type.insert(std::make_pair("NAMES", NAMES));
	_command_to_type.insert(std::make_pair("LIST", LIST));
	_command_to_type.insert(std::make_pair("INVITE", INVITE));
	_command_to_type.insert(std::make_pair("KICK", KICK));
	_command_to_type.insert(std::make_pair("MODE", MODE));
	_command_to_type.insert(std::make_pair("PRIVMSG", PRIVMSG));
	_command_to_type.insert(std::make_pair("NOTICE", NOTICE));
	_command_to_type.insert(std::make_pair("PING", PING));
	_command_to_type.insert(std::make_pair("PONG", PONG));
}

IRCMessage::~IRCMessage()
{
}

std::string	IRCMessage::reply_servername_prefix(const std::string& numeric_reply)
{
	std::string nick = _client->get_names().nick;
	return (":ircserver " + numeric_reply + " " + (nick.empty() ? "*" : nick));
}

std::string	IRCMessage::reply_nickmask_prefix(const std::string& command)
{
	std::string str;
	str = str + ":" + _client->get_nickmask() + " " + command;
	return (str);
}

std::string IRCMessage::err_no_such_nick(const std::string& nickname)
{
	return (reply_servername_prefix("401/") + nickname + " : No such nick" + IRCMessage::endl);
}

std::string IRCMessage::err_no_such_channel(const std::string& ch_name)
{
	return (reply_servername_prefix("403/") + ch_name + " : No such channel" + IRCMessage::endl);
}

std::string IRCMessage::err_cannot_send_to_channel(const std::string& ch_name, char mode)
{
	return (reply_servername_prefix("404/") + ch_name + " : cannot send to channel (+" + mode + ')' + IRCMessage::endl);
}

std::string IRCMessage::err_too_many_channels(const std::string& ch_name)
{
	return (reply_servername_prefix("405/") + ch_name + " : you have joined too many channels" + IRCMessage::endl);
}

std::string IRCMessage::err_too_many_targets(const std::string& target)
{
	return (reply_nickmask_prefix("407/") + target + " : Duplicate recipients. No message delivered" + IRCMessage::endl);
}

std::string IRCMessage::err_no_recipient()
{
	return (reply_servername_prefix("411/") + " : No recipient given (" + _request->command + ")" + IRCMessage::endl);
}

std::string IRCMessage::err_no_text_to_send()
{
	return (reply_servername_prefix("412/") + " : No text to send" + IRCMessage::endl);
}

std::string IRCMessage::err_unknown_command()
{
	return (reply_servername_prefix("421/") + _request->command + " :Unknown command" + IRCMessage::endl);
}

std::string IRCMessage::err_file_error(const std::string& file_op, const std::string& file)
{
	return (reply_servername_prefix("424/") + " : File error doing " + file_op + " on " + file + IRCMessage::endl);
}

std::string IRCMessage::err_no_nickname_given()
{
	return (reply_servername_prefix("431/") + " : No nickname given" + IRCMessage::endl);
}

std::string IRCMessage::err_erroneus_nickname(const std::string& nickname)
{
	return (reply_servername_prefix("432/") + nickname + " :Erroneus nickname" + IRCMessage::endl);
}

std::string IRCMessage::err_nickname_in_use(const std::string& nickname)
{
	return (reply_servername_prefix("433/") + nickname + " :Nickname is already in use" + IRCMessage::endl);
}

std::string IRCMessage::err_user_not_in_channel(const std::string& nickname, const std::string& channel)
{
	return (reply_servername_prefix("441/") + nickname + " " + channel + " : They aren't on that channel" + IRCMessage::endl);
}

std::string IRCMessage::err_not_on_channel(const std::string& channel)
{
	return (reply_servername_prefix("442/") + channel + " :You're not on that channel" + IRCMessage::endl);
}

std::string IRCMessage::err_user_on_channel(const std::string& username, const std::string& channel)
{
	return (reply_servername_prefix("443/") + username + " " + channel + " :is already on channel" + IRCMessage::endl);
}

std::string IRCMessage::err_not_registered()
{
	return (reply_servername_prefix("451/") + " :You have not registered" + IRCMessage::endl);
}

std::string IRCMessage::err_need_more_params()
{
	return (reply_servername_prefix("461/") + _request->command + " :Not enough parameters" + IRCMessage::endl);
}

std::string IRCMessage::err_already_registred()
{
	return (reply_servername_prefix("462/") + " : You may not reregister" + IRCMessage::endl);
}

std::string IRCMessage::err_passwd_mismatch()
{
	return (reply_servername_prefix("464/") + " : Password incorrect" + IRCMessage::endl);
}

std::string IRCMessage::err_channel_is_full(const std::string& channel)
{
	return reply_servername_prefix("471") + channel + " :Cannot join channel (+l)" + IRCMessage::endl;
}

std::string IRCMessage::err_unknown_mode(char mode)
{
	return (reply_servername_prefix("472") + " " + mode + " :Unknown MODE flag" + IRCMessage::endl);
}

std::string IRCMessage::err_invite_only_channel(const std::string& channel)
{
	return (reply_servername_prefix("473") + " " + channel + " :Cannot join channel (+i)" + IRCMessage::endl);
}

std::string IRCMessage::err_chanoprivs_needed(const std::string& channel)
{
	return (reply_servername_prefix("482") + " " + channel + " :You're not channel operator" + IRCMessage::endl);
}

std::string IRCMessage::err_u_mode_unknown_flag()
{
	return (reply_servername_prefix("501") + " :Unknown MODE flag" + IRCMessage::endl);
}

std::string IRCMessage::err_users_dont_match(const std::string& action)
{
	return reply_servername_prefix("502") + " :Can't " + action + " modes for other users" + IRCMessage::endl;
}

std::string IRCMessage::rpl_list(const std::string channel, const std::string& visible, const std::string topic)
{
	return (reply_servername_prefix("322") + " " + channel + " " + visible + " :" + topic + IRCMessage::endl);
}

std::string IRCMessage::rpl_listend()
{
	return reply_servername_prefix("323") + " :End of LIST" + IRCMessage::endl;
}

std::string IRCMessage::rpl_channel_mode_is(const std::string& channel, const std::string& mode)
{
	return reply_servername_prefix("324") + " " + channel + " +" + mode + IRCMessage::endl;
}

std::string IRCMessage::rpl_notopic(const std::string& channel)
{
	return reply_servername_prefix("331") + " " + channel + " :No topic is set" + IRCMessage::endl;
}

std::string IRCMessage::rpl_topic(const std::string& channel, const std::string& topic)
{
	return reply_servername_prefix("332") + " " + channel + " :" + topic + IRCMessage::endl;
}

//https://gist.github.com/proxypoke/2264878

std::string IRCMessage::rpl_inviting(const std::string& nickname, const std::string& channel)
{
	return (reply_servername_prefix("341") + " " + nickname + " " + channel + IRCMessage::endl);
}

std::string IRCMessage::rpl_namereply(const std::string& str)
{
	return reply_servername_prefix("353") + " " + str + IRCMessage::endl;
}

std::string IRCMessage::rpl_endofnames(const std::string& channel)
{
	return reply_servername_prefix("366") + " " + channel + " :End of NAMES list" + IRCMessage::endl;
}

std::string IRCMessage::rpl_user_mode_is()
{
	return reply_servername_prefix("221") + " +" + IRCMessage::endl;
}

std::string IRCMessage::rpl_welcome()
{
	return reply_servername_prefix("001")
           + " Welcome to Internet Relay Network\n" + _client->get_nickmask()
           + IRCMessage::endl;
}

//command reply
std::string IRCMessage::cmd_quit_reply(const std::string& reason)
{
	return reply_nickmask_prefix("QUIT") + " :" + reason + IRCMessage::endl;
}

std::string IRCMessage::cmd_part_reply(const std::string& channel)
{
	std::string param;

    if (_request->parameter.size() == 2)
        param = _request->parameter[1];
    else
        param = _client->get_names().nick;
    return reply_nickmask_prefix(_request->command) + " " + channel + " :"
           + param + IRCMessage::endl;
}

std::string IRCMessage::cmd_message_reply(const std::string& target)
{
	return reply_nickmask_prefix(_request->command) + " " + target + " :"
           + _request->parameter[1] + IRCMessage::endl;
}

std::string IRCMessage::cmd_invite_reply(const std::string& nick, const std::string& channel)
{
	return reply_nickmask_prefix(_request->command) + " " + nick + " " + channel
           + IRCMessage::endl;
}

std::string IRCMessage::cmd_kick_reply(const std::string& channel, const std::string& nick, const std::string& oper_nick)
{
	return reply_nickmask_prefix(_request->command) + " " + channel + " " + nick
           + " " + oper_nick + IRCMessage::endl;
}
std::string IRCMessage::cmd_nick_reply(const std::string& nick)
{
	return reply_nickmask_prefix(_request->command) + " " + nick + IRCMessage::endl;
}

std::string IRCMessage::cmd_join_reply(const std::string& channel)
{
	return reply_nickmask_prefix(_request->command) + " " + channel + IRCMessage::endl;
}

std::string IRCMessage::cmd_mode_reply(const std::string& channel, const std::string& mode)
{
	return reply_nickmask_prefix(_request->command) + " " + channel + " " + mode + IRCMessage::endl;
}

std::string IRCMessage::cmd_topic_reply()
{
	return reply_nickmask_prefix(_request->command) + " " + _request->parameter[0] + " :" + _request->parameter[1] + IRCMessage::endl;
}

std::string IRCMessage::cmd_pong_reply()
{
	return std::string("PONG ") + NAME_SERVER + IRCMessage::endl;
}

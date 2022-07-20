#include "../includes/ircserver.hpp"
#include "../includes/ircmessage.hpp"
#include <sstream>


IRCCommand::t_cstr_vector
    IRCCommand::split(const std::string& params, char delimiter)
{
    IRCCommand::t_cstr_vector splited;
    std::istringstream  iss(params);
    std::string         element;

    while (std::getline(iss, element, delimiter))
        splited.push_back(element);
    return splited;
}

e_type
    IRCCommand::get_type(const std::string& command)
{
    if (_command_to_type.count(command))
        return _command_to_type[command];
    return (UNKNOWN);
}

static inline bool
    is_special(const char c)
{
    return std::memchr(SPECIALCHAR, c, 9);
}

e_result
    IRCCommand::m_is_valid(e_type type)
{
    if (type == NICK)
    {
        if (NICK_LENGTH_MAX < _target->length())
            return ERROR;
        if (!std::isalpha((*_target)[0]))
            return ERROR;
        for (size_t index = 1; index < _target->length(); ++index)
            if (!std::isalpha((*_target)[index])
                && !std::isdigit((*_target)[index])
                && !is_special((*_target)[index]))
                return ERROR;
    }
    if (type == CHANNEL_PREFIX)
    {
        if ((*_target)[0] != CHANNEL_PREFIX)
            return ERROR;
    }
    if (type == CHANNEL_NAME)
    {
        if (CHANNEL_LENGTH_MAX < _target->length())
            return ERROR;
        for (size_t index = 0; index < _target->length(); ++index)
            if (std::memchr(CHSTRING, (*_target)[index], 5))
                return ERROR;
    }
    return OK;
}

e_result
    IRCCommand::m_to_client(std::string message)
{
    _to_client->buffer.append(message);
    return ERROR;
}

void
    IRCCommand::m_to_client(IRCClient& client, const std::string& message)
{
    client.get_buffers().to_client.buffer.append(message);
    _ircserver->toggle(client, EVFILT_READ);
}
void
    IRCCommand::m_to_channel(const std::string& message)
{
    IRCChannel::t_citer_member iter = _channel->get_members().begin();
    IRCChannel::t_citer_member end  = _channel->get_members().end();
    if (_channel->get_operator() && _channel->get_operator() != _client)
        m_to_client(*_channel->get_operator(), message);
    for (; iter != end; ++iter)
        if (*iter != _client)
            m_to_client(**iter, message);
}

void
    IRCCommand::m_to_channels(const std::string& message)
{
    IRCClient::t_citer   iter = _client->get_channels().begin();
    IRCClient::t_citer   end  = _client->get_channels().end();
    std::set<IRCClient*> check;

    for (_channel = *iter; iter != end; _channel = *(++iter))
    {
        IRCChannel::t_citer_member users = _channel->get_members().begin();
        IRCChannel::t_citer_member u_end = _channel->get_members().end();
        for (; users != u_end; ++users)
            if (!check.count(*users) && *users != _client)
            {
                check.insert(*users);
                IRCCommand::m_to_client(**users, message);
            }
    }
}

void IRCCommand::m_disconnect(const std::string& message)
{
    _ircserver->irc_disconnect(message);
}

void IRCCommand::parse_parameter(std::vector<std::string>& parameter)
{
    for (_offset = 0;
         (_index = _buffer.find_first_not_of(' ')) != (int)std::string::npos;)
    {
        _offset = _buffer.find_first_of(' ', _index);
        if ((_offset != (int)std::string::npos) && _buffer[_index] != ':')
            parameter.push_back(_buffer.substr(_index, _offset - _index));
        else
        {
            if (_buffer[_index] == ':')
                ++_index;
            parameter.push_back(_buffer.substr(_index));
            break;
        }
        _buffer.erase(0, _offset);
    }
    _buffer.clear();
}

void
    IRCCommand::parse_command(std::string& command)
{
    for (_offset = 0; (command[_offset] != ' ' && command[_offset] != '\0');
         ++_offset)
        if ((unsigned)command[_offset] - 'a' < 26)
            command[_offset] ^= 0b100000;
    _buffer = command.substr(_offset);
    command.erase(_offset);
}

void
    IRCCommand::parse_request(IRCClient::t_request& request)
{
    _request = &request;
    if (_request->command.size() && (_request->command.front() == ':'))
    {
        _request->command.erase(0, _request->command.find_first_of(' '));
        _request->command.erase(0, _request->command.find_first_not_of(' '));
    }
    if (_request->command.size())
    {
        parse_command(_request->command);
        if (_buffer.size())
            parse_parameter(_request->parameter);
    }
}

void
    IRCCommand::registration()
{
    _map.client[_client->get_names().nick] = _client;
    m_to_client(rpl_welcome());
    log::print() << _client->get_names().nick << " is registered" << log::endl;
}

IRCCommand::~IRCCommand()
{
}

IRCCommand::IRCCommand()
{
    _commands.push_back(&IRCCommand::empty);
    _commands.push_back(&IRCCommand::pass);
    _commands.push_back(&IRCCommand::nick);
    _commands.push_back(&IRCCommand::user);
    _commands.push_back(&IRCCommand::quit);
    _commands.push_back(&IRCCommand::join);
    _commands.push_back(&IRCCommand::part);
    _commands.push_back(&IRCCommand::topic);
    _commands.push_back(&IRCCommand::names);
    _commands.push_back(&IRCCommand::list);
    _commands.push_back(&IRCCommand::invite);
    _commands.push_back(&IRCCommand::kick);
    _commands.push_back(&IRCCommand::mode);
    _commands.push_back(&IRCCommand::privmsg);
    _commands.push_back(&IRCCommand::notice);
    _commands.push_back(&IRCCommand::ping);
    _commands.push_back(&IRCCommand::pong);
    _commands.push_back(&IRCCommand::unknown);
    _commands.push_back(&IRCCommand::unregistered);
    m_mode_initialize();
}

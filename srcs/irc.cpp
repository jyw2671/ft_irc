#include "irc.hpp"

IRC::~IRC()
{
}

//irc command
IRC::IRC() : endl("\r\n")
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
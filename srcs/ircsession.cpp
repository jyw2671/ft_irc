#include "ft_irc.hpp"
#include "ircserver.hpp"
#include "ircsession.hpp"
#include "irc_exception.hpp"
#include "ircnumericmessage.hpp"
#include "ircmessage.hpp"
#include <stdexcept>
#include <cctype>
#include <algorithm>
#include <map>
#include <sstream>
#include <vector>
#include <set>
#include <ctime>

IRCSession::IRCSession(IRCServer* server, Channel* channel, int socketfd, int sokcetId, const std::string& addr)
 : Session(channel, socketfd, sokcetId, addr), _nickname(), _username(), _flag(0), _password(), _closeReason(), _channels(), _pingState(PinState_Active), _lastPingTime(std::time(NULL)), _lastPingWord(), _server(server), _registerFlag(0) {}
 
IRCSession::~IRCSession() {}

void IRCSession::Process(const std::string& line)
{
    //부모 클래스의 프로세스 실행
    Session::Process(line);
    
    //IRC 명령어 처리
    try
    {
        IRCMessage msg = IRCMessage::parse(line);
        if (msg.IsEmpty())
            return ;
        const std::string& cmd = msg.GetCommand();
        if (cmd == "NICK")
            _server->cmdNick(*this, msg);
        else if (cmd == "USER")
            _server->cmdUser(*this, msg);
        else if (cmd == "PASS")
            _server->cmdPass(*this, msg);
        else if (cmd == "QUIT")
            _server->cmdQuit(*this, msg);
        else if (cmd == "PING")
            cmdPing(msg);
        else if (cmd == "PONG")
            cmdPong(msg);
        else
        {
            //Login 후 사용가능한 명령어
            if (!IsFullyRegistered())
                throw irc_exception(ERR_NOTREGISTERED, cmd, "You are not register");
            
            if (cmd == "JOIN")
                _server->cmdJoin(*this, msg);
            else if (cmd == "PART")
                _server->cmdPart(*this, msg);
            else if (cmd == "PART")
                _server->cmdPart(*this, msg);
            else if (cmd == "NAMES")
                _server->cmdNames(*this, msg);
            else if (cmd == "PRIVMSG" || cmd == "NOTICE")
                _server->cmdPrivMsg(*this, msg, cmd);
            else if (cmd == "TOPIC")
                _server->cmdTopic(*this, msg);
            else if (cmd == "PART")
                _server->cmdPart(*this, msg);
            else if (cmd == "MODE")
                _server->cmdMode(*this, msg);
            else if (cmd == "KILL")
                _server->cmdKill(*this, msg);
            else if (cmd == "MOTD")
                _server->cmdMOTD(*this, msg);
            else
                throw irc_exception(ERR_UNKNOWNCOMMAND, cmd, "Unknown Command");
        }
        catch (const irc_exception& irex)
        {
            SendMessage(irex.message());
        }
        catch (const std::exception& ex)
        {
            //std::exception 발생 시 오류 전송 후 세션 종료 
            Close(ex.what());
        }
    }
}


//setter
void IRCSession::SetNickname(const std::string& nickname) { _nickname = nickname; }
void IRCSession::SetUsername(const std::string& username) { _username = username; }
void IRCSession::SetPassword(const std::string& password) { _password = password; }
int IRCSession::SetFlag(const ModeChange& modeChange)
{
    const bool adding = modeChange.sign == '+';
    
    int modeFlag = 0;
    if (modeChange.ch == 'o')
        modeFlag = FLAG_OG;
    if (adding)
    {
        if (HasFlag(modeFlag))
            return (ModeChange::CHANGEMODE_NOTADDECTED);
        _flag |= modeFlag;
    }
    else
    {
        if (!HasFlag(modeFlag))
            return (ModeChange::CHANGEMODE_NOTADDECTED);
        _flag &= ~modeFlag;
    }
    return (ModeChange::CHANGEMODE_SUCCESS);
}

//getter
const std::string& IRCSession::GetNickname() const { return (_nickname); }
const std::string& IRCSession::GetUsername() const { return (_username); }
const std::string& IRCSession::GetPassword() const { return (_password); }


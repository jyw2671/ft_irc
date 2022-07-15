#ifndef IRCSESSION_HPP
#define IRCSESSION_HPP

#include "irccompare.hpp"
#include "modelist.hpp"
#include <string>
#include <set>
#include <ctime>

class IRCServer;
class IRCMessage;

//IRC의 로직을 수행할 세션 클래스
//Session은 libserv에 존재하는 Session을 이용
//libserv에는 kqueue로 소켓을 만든 부분을 분리, 어플리케이션 계층 제외

class IRCSession : public Session
{
    private:
        enum { PinState_Active, PingState_Idle };
        
        std::string _nickname;
        std::string _username;
        std::string _password;
        std::string _closeReason;
        int _flag;
        
        std::set<const std::string, IRCComparer> _channels;
        
        int _pingState;
        std::time_t _lastPingTime;
        std::string _lastPingWord;
        
        IRCSession();
        IRCSession(const IRCSession&);
        IRCSession& operator=(const IRCSession&);
        
        void cmdPing(const IRCMessage& msg);
        void cmdPong(const IRCMessage& msg);
        void UpdateActive();
        
    protected:
        IRCServer* _server;
        int _registerFlag;
    
    public:
        enum { FLAG_NICKNAME = 1 << 0, FLAG_USERNAME = 1 << 1, FLAG_OG = 1 << 2 };
        
        IRCSession(IRCServer* server, Channel* channel, int socketfd, int socketId, const std::string& addr);
        virtual ~IRCSession();
        
        void    Process(const std::string& line);
        void    SendMessage(const IRCMessage& msg);
        void    MessageToNeighbor(const IRCMessage& msg, IRCSession* except);

        void                SetNickname(const std::string& nickname);
        const std::string&  GetNickname() const;
        void                SetUsername(const std::string& username);
        const std::string&  GetUsername() const;
        void                SetPassword(const std::string& password);
        const std::string&  GetPassword() const;
        
        inline bool         HasFlag(int flag) const { return (_flag & flag) == flag; }
        int                 SetFlag(const ModeChange& modeChange);
        const std::string   GetMask() const;
        const std::string   GetEmail() const;
        const std::string&  GetCloseReason() const;
    
        void    RegisterStep(int flag);
        bool    HasRegisterFlag(int flag) const;
    	int		HasOperatorFlag(std::string str);
        bool    IsFullyRegistered() const;
    
        virtual void    Close();
        void            Close(const std::string& reason);
    
        bool    AddChannel(const std::string& name);
        bool    RemoveChannel(const std::string& name);
        bool    IsJoinedChannel(const std::string& name) const;
        size_t  GetJoinedChannelNum() const;
    
        virtual void    CheckActive();
}
#endif
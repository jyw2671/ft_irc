#ifndef IRCSERVER_HPP
#ifndef IRCSERVER_HPP

//TODO
//string.hpp + shared_ptr.hpp -> utils.hpp
#include <iosfwd>
#include <map>
#include <set>
#include "string.hpp"
#include "ircchannel.hpp"
#include "shared_ptr.hpp"
#include "timerhandler.hpp"
#include "modelist.hpp"

class IRCSession;
class IRCMessage;
class IRCBot;

class IRCServer : public ITimerHandler
{
    public:
        typedef std::map<const std::string, IRCSession*, IRCComparer> ClientMap;
        typedef std::map<const std::string, SharedPtr<IRCChannel> IRCComparer> ChannelMap;
    
    private:
        struct ModeTarget
        {
            bool isChannel;
            IRCChannel* channel;
            IRCSession* session;
            
            inline ModeTarget(bool isChannel) : isChannel(isChannel) {}
        }
        
        const std::string   _password;
        ClientMap   _clients;
        ChannelMap  _channels;
        
        IRCServer(const IRCServer&);
        IRCServer& operator= (const IRCServer&);
    public :
        IRCServer(const std::string & password);
        ~IRCServer();
        
        //IRC command
        void cmdNick(IRCSession& session, IRCMessage& msg);
        void cmdUser(IRCSession& session, IRCMessage& msg);
        void cmdPass(IRCSession& session, IRCMessage& msg);
        void cmdQuit(IRCSession& session, IRCMessage& msg);
        void cmdJoin(IRCSession& session, IRCMessage& msg);
        void cmdPart(IRCSession& session, IRCMessage& msg);
        void cmdNames(IRCSession& session, IRCMessage& msg);
        void cmdPrivMsg(IRCSession& session, IRCMessage& msg, const std::string& cmd);
        void cmdTopic(IRCSession& session, IRCMessage& msg);
        void cmdList(IRCSession& session, IRCMessage& msg);
        void cmdMode(IRCSession& session, IRCMessage& msg);
        void cmdKill(IRCSession& session, IRCMessage& msg);
        void cmdMOTD(IRCSession& session);
        
        void UnreisterNickName(const std::string& nick);
        void JoinChannel(IRCSession& session, const std::string& chanName);
        void LeaveChannel(IRCSession& session, const std::string& chanName, const std::string& cmd, const std::string& message = "")
        bool    IsPassowrdMatch(const std::string& password) const;
        void    onTimer();
        size_t  GetInterval() const;
        
        IRCSession* FicdByNick(const std::string& nick) const;
        IRCChannel* FindChannel(const std:string& channel);
        
        void RegisterBot(IRCBot& bot);
        
        template<typename ChannelNameIterator>
        void GatherNeighbors(std::set<IRCSession*>& neighbors, ChannelNameIterator first, ChannelNameIterator last, IRCSession* except = NULL)
        {
            for (; first != last; ++first)
            {
                ChannelMap::Iterator chanIt = _channel.find(*first);
                if (chanIt == _channel.end())
                    continue;
                IRCChannel* channel = chanIt->second.Load();
                channel->GatherParticipants(neighbors, except);
            }
        }

}
#endif
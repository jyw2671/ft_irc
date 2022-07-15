#include IRCSESSIONFACTORY_HPP
#define IRCSESSIONFACTORY_HPP

#include "sessionfactory.hpp"

// IRC세션을 채널에서 생성시켜줄 팩토리 클래스
class IRCServer;

class IRCSessionFactory : public ISessionFactory
{
    private:
        IRCServer*  _server;
        
        IRCSessionFactory();
        IRCSessionFactory(const IRCSessionFactory&);
        IRCSessionFactory& operator= (const IRCSessionFactory&);
    public:
        IRCSessionFactory(IRCServer* server);
        ~IRCSessionFactory();
        
        virtual Session* CreateSession(Channel* channel, int socketfd, int socketId, const std::string& addr);
}
#endif
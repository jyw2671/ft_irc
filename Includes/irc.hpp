#ifndef IRC_HPP
#define IRC_HPP

#include "channel.hpp"
#include "client.hpp"
#include "log.hpp"
#include "resources.hpp"
#include "ircd.hpp"

class Server;

class IRC
{
    public:
        typedef std::map<e_type, std::string>   t_map_irc;
        typedef std::map<std::string, e_type>   t_map_type;
        typedef std::map<std::string, Client*>  t_map_client;
        typedef std::map<std::string, Channel*> t_map_channel;
        typedef t_map_client::const_iterator    t_iter_cl;
        typedef t_map_channel::const_iterator   t_iter_ch;
        
        IRC();
        ~IRC();
    private:
        IRC(const IRC&);
        IRC& operator=(const IRC&);

        std::string       reply_servername_prefix(const std::string&);
        std::string       reply_nickmask_prefix(const std::string&);
        const std::string endl;
    
    protected:
        Server*             _ft_ircd;
        t_map_irc            _type_to_command;
        t_map_type           _command_to_type;
        Client*              _client;
        Channel*             _channel;
        Client::t_requests*  _requests;
        Client::t_request*   _request;
        Client::t_to_client* _to_client;
        std::string          _password;
        bool                 _ascii[127];
}
#endif
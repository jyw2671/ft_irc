#ifndef IRCCLIENT_HPP
#define IRCCLIENT_HPP

#include "ircutils.hpp"
#include "ircchannel.hpp"
#include "ircserver.hpp"
#include <arpa/inet.h>
#include <netinet/in.h>

class IRCChannel;

class IRCClient
{
	public:
		typedef struct s_names
		{
			std::string	nick;
			std::string	user;
			std::string	host;
			std::string	server;
			std::string	real;
		} t_names;

		typedef struct s_request
		{
			std::string	command;
			std::vector<std::string>	parameter;
			e_type	type;
			s_request(std::string line, e_type type) : command(line), type(type) {};
		} t_request;

		typedef struct s_requests
		{
			IRCClient*	from;
			std::queue<IRCClient::t_request> queue;
		} t_requests;

		typedef struct s_to_client
		{
			int	offset;
			std::string buffer;
		} t_to_client;

		typedef struct s_buffers
		{
			int	offset;
			std::string	buffer;
			t_requests	requests;
			t_to_client	to_client;
		} t_buffers;

		typedef union
		{
			struct
			{
				unsigned char pass : 1;
				unsigned char nick : 1;
				unsigned char user : 1;
			};
			unsigned char regsistered;
		} t_status;

		typedef std::set<IRCChannel*>::const_iterator t_citer;

	private:
		sockaddr_in	_addr;
		int	_fd;
		std::set<IRCChannel*> _channels;

	protected:
		t_status _status;
		t_names	_names;
		t_buffers _buffers;

	public:
		IRCClient(sockaddr_in client_addr, int client_fd);
		IRCClient();
		~IRCClient();

		//getter
		sockaddr_in	get_addr();
		int			get_fd();
		char*		get_IP();
		const t_names&	get_names() const;
		t_buffers&		get_buffers();
		const std::set<IRCChannel*>&	get_channels() const;
		bool		get_status(e_type);
		std::string	get_nickmask();

		//setter
		void set_nickname(const std::string& nickname);
		void set_username(const std::string& username);
		void set_realname(const std::string& realname);
		void set_status(e_type);

		bool is_registered() const;
		bool is_joined(IRCChannel* channel);

		void joined(IRCChannel* channel);
		void parted(IRCChannel* channel);
};

#endif

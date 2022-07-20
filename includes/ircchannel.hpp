#ifndef IRCCHANNEL_HPP
#define IRCCHANNEL_HPP

#include "ircutils.hpp"

class IRCClient;

class IRCChannel
{
	public:
		typedef std::string						t_str_info;
		typedef std::vector<IRCClient*>			t_vector_memver;
		typedef std::set<IRCClient*>			t_set_invitee;
		typedef t_vector_memver::const_iterator	t_citer_member;

		typedef union
		{
			struct
			{
				unsigned char negative : 1;
				unsigned char positive : 1;
			};
			unsigned char state;
		} t_sign;

		typedef union
		{
			struct
			{
				unsigned char invite : 1;
				unsigned char topic : 1;
				unsigned char nomsg : 1;
			};
			unsigned char state;
		} t_status;

		typedef struct s_reserved
		{
			t_sign	sign;
			t_status	flag;
		} t_reserved;

	private:
		IRCChannel();
		IRCChannel(const IRCChannel&);
		IRCChannel& operator=(const IRCChannel&);
		bool	m_set_status(const bool&, unsigned char&);
		const IRCChannel::t_citer_member	find(IRCClient* client);

		std::string		_name;
		std::string		_topic;
		IRCClient*		_operator;
		t_vector_memver	_members;
		t_status		_status;
		t_reserved		_reserved;
		t_set_invitee	_invitees;
	public:
		IRCChannel(const std::string& name, IRCClient* client);
		~IRCChannel();

		//getter
		const std::string&		get_name() const;
		const std::string&		get_topic() const;
		const t_vector_memver&	get_members();
		bool					get_status(e_type type);
		std::string				get_status();
		IRCClient*				get_operator();

		//setter
		void	set_name(const std::string& name);
		void	set_topic(const std::string& topic);
		void	set_status(e_type type, bool state);
		void	set_status(std::string& status);

		void	reserve_flags(const char c);
		void	reserve_sign(const char c);
		void	reserve_clear();

		bool	is_empty();
		bool	is_full();
		bool	is_operator(IRCClient* client);
		bool	is_joined(IRCClient* client);
		bool	is_invited(IRCClient* client);
		bool	is_signed();
		bool	is_reserved();

		void	join(IRCClient* client);
		void	part(IRCClient* client);
		void	invitation(IRCClient* client);
};


#endif

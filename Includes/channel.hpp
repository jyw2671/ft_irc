#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "client.hpp"
#include "log.hpp"
#include "utils.hpp"

class Client;

class Channel
{
	public:
		typedef std::string		t_str_info;
		typedef std::vector<Client*>	t_vector_memver;
		typedef std::set<Client*>		t_set_invitee;
		typedef t_vector_memver::const_iterator t_citer_member;

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
		Channel();
		Channel(const Channel&);
		Channel& operator=(const Channel&);
		bool	m_set_status(const bool&, unsigned char&);
		const Channel::t_citer_member	find(Client* client);

		std::string	_name;
		std::string	_topic;
		Client*		_operator;
		t_vector_memver	_members;
		t_status	_status;
		t_reserved	_reserved;
		t_set_invitee	_invitees;
	public:
		Channel(const std::string& name, Client* client);
		~Channel();

		//getter
		const std::string&	get_name() const;
		const std::string&	get_topic() const;
		const t_vector_memver&	get_members();
		bool	get_status(e_type type);
		std::string	get_status();
		Client*	get_operator();

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
		bool	is_operator(Client* client);
		bool	is_joined(Client* client);
		bool	is_invited(Client* client);
		bool	is_signed();
		bool	is_reserve();

		void	join(Client* client);
		void	part(Client* client);
		void	invitation(Client* client);
};


#endif

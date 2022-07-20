#ifndef IRCCOMMAND_HPP
#define IRCCOMMAND_HPP

#include "ircmessage.hpp"
#include "ircutils.hpp"

class IRCCommand : public IRCMessage
{
	public:
		typedef struct s_map
		{
				t_map_client  client;
				t_map_channel channel;
		} t_map;

		typedef std::vector<void (IRCCommand::*)()>  t_commands_irc;
		typedef std::vector<const std::string> t_cstr_vector;
		typedef t_cstr_vector::iterator        t_iter;

	private:
		const std::string*   _target;
		const std::string*   _target_sub;
		std::string          _buffer;
		IRCClient*          _fixed;
		int                  _offset;
		int                  _index;
		static t_cstr_vector split(const std::string& params, char delimiter);
		e_result             m_is_valid(e_type);
		e_result             m_to_client(std::string);
		void                 m_to_client(IRCClient&, const std::string&);
		void                 m_to_channel(const std::string&);
		void                 m_mode_valid(const char);
		void                 m_mode_invalid(const char);
		void                 m_mode_sign(const char);
		void                 m_mode_initialize();
		void                 m_bot_initialize();
		void                 m_disconnect(const std::string&);

	protected:
		void           m_to_channels(const std::string&);
		e_type         get_type(const std::string& command);
		void           registration();
		void           parse_parameter(std::vector<std::string>&);
		void           parse_command(std::string&);
		void           parse_request(IRCClient::t_request&);
		e_result       parse_flag(const std::string&);
		t_commands_irc _commands;
		t_map          _map;
		void (IRCCommand::*_modes[128])(const char);

	private:
		e_result m_pass();
		e_result m_nick();
		e_result m_user();
		e_result m_join(e_phase, IRCChannel* = nullptr);
		e_result m_part(e_phase);
		e_result m_topic();
		e_result m_names();
		e_result m_list();
		e_result m_invite();
		e_result m_kick(e_phase);
		e_result m_mode(e_phase);
		e_result m_privmsg(e_phase);

	protected:
		void empty(){};
		void pass();
		void nick();
		void user();
		void quit();
		void join();
		void part();
		void topic();
		void names();
		void list();
		void invite();
		void kick();
		void mode();
		void privmsg();
		void notice();
		void ping();
		void pong();
		void unknown()
		{
				m_to_client(err_unknown_command());
		};
		void unregistered()
		{
				m_to_client(err_not_registered());
		};
		IRCCommand();
		~IRCCommand();
};

#endif

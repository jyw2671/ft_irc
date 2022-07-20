#include "../../includes/irccommand.hpp"

e_result
    IRCCommand::m_pass()
{
    if (_request->parameter.empty())
        return m_to_client(err_need_more_params());
    if (_client->is_registered())
        return m_to_client(err_already_registred());
    if (_request->parameter[0] != _password)
        return m_to_client(err_passwd_mismatch());
    return OK;
}

void
    IRCCommand::pass()
{
    if (m_pass() == ERROR)
        return;
    _client->set_status(PASS);
}

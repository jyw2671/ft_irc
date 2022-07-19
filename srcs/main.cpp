#include "../includes/ft_irc.hpp"

int main(int argc, char *argv[])
{
	/**
	 * @brief argument error
	 *
	 */
	if (argc < 3)
	{
		std::cerr << "usage: " << argv[0] << "<port> <password>" << std::endl;
		return (1);
	}

	/**
	 * @brief port exception and irc failed creating server exception
	 *
	 * 입력받은 포트 번호를 정수로 변환하고, 변화하지 못하거나 범위를 벗어나면 예외발생.
	 */
	try
	{
		int port;

		try
		{
			port = (unsigned int)atoi(argv[1]);
		}
		catch (const std::exception& ex)
		{
			throw std::runtime_error("Typed port number is not integer.");
		}
		if (port < 1 || port > 65535)
			throw std::runtime_error("Not valid port range");
		/**
		 * @brief IRCServer is Server
		 *
		 * IRCserver create constructor
		 * IRCserver start(run)
		 */
		IRCServer ircserver(port, argv[2]);
		ircserver.start();
	}
	catch(const std::exception& ex)
	{
		std::cerr << "Failed to start server" << std::endl;
		return (1);
	}
}

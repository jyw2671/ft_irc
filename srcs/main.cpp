#include "ft_irc.hpp"

int main(int argc, char *argv[])
{

    if (argc < 3)
    {
        std::cerr << "usage: " << argv[0] << "<port> <password>" << std::endl;
        return (1);
    }
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
        FT_IRC ft_irc(port, argv[2]);
        ft_irc.start();
    }
    catch(const std::exception& ex)
    {
        std::cerr << "Failed to start server" << std::endl;
        return (1);
    }
}

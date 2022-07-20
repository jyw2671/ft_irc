NAME    	= ircserv
CXX      	= c++
CPPFLAGS	= -Wall -Wextra -Werror -std=c++98
RM			= rm -rf

srcs		= main.cpp\
			  ircserver.cpp\
			  ircchannel.cpp\
			  ircclient.cpp\
			  irccommand.cpp\
			  ircevent.cpp\
			  ircsocket.cpp\
			  ircmessage.cpp\
			  irclog.cpp\
			  ircd/channel_operations.cpp\
			  ircd/connection_messages.cpp\
			  ircd/sending_messages.cpp\
			  ircd/mode.cpp

SRCS    	= $(srcs:%=srcs/%)

OBJS		= $(SRCS:srcs/%.cpp=objs/%.o)

all     	: $(NAME)

objs/%.o   	: srcs/%.cpp
	@mkdir -p $(dir ./objs/$*)
	$(CXX) $(CPPFLAGS) -c $< -o $@

$(NAME) 	: $(OBJS)
	$(CXX) $(CPPFLAGS) $(OBJS) -o $(NAME)

clean   	:
	$(RM) ./objs

fclean    	: clean
	$(RM) $(NAME)

re			: fclean all

.PHONY		: all clean fclean re

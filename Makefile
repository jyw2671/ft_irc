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
			  ircd/invite.cpp\
			  ircd/join.cpp\
			  ircd/kick.cpp\
			  ircd/list.cpp\
			  ircd/mode.cpp\
			  ircd/names.cpp\
			  ircd/nick.cpp\
			  ircd/notice.cpp\
			  ircd/part.cpp\
			  ircd/pass.cpp\
			  ircd/privmsg.cpp\
			  ircd/quit.cpp\
			  ircd/topic.cpp\
			  ircd/user.cpp\
			  ircd/ping.cpp

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

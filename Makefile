SRCS		=	srcs/Polls.cpp				\
				srcs/Server.cpp				\
				srcs/Meta.cpp				\
				srcs/Request.cpp			\
				srcs/RequestCollector.cpp	\
				srcs/Response.cpp			\
				srcs/Maintainer.cpp			\
				srcs/CGI.cpp				\
				srcs/utils/utils.cpp		\
				srcs/webserv.cpp

OBJS		=	$(SRCS:.cpp=.o)

HEAD		=	srcs/

UT_HEAD		=	srcs/utils/

NAME		=	webserv

GCC			=	c++

FLAGS		=	-Wall -Werror -Wextra -std=c++98

RM			=	rm -f

%.o:	%.cpp $(wildcard $(HEAD)/*.hpp) $(wildcard $(UT_HEAD)/*.hpp)
		$(GCC) $(FLAGS) -c -I$(HEAD) -I$(UT_HEAD) $< -o $@ 

$(NAME):	$(HEADS) $(OBJS)
			$(GCC) $(FLAGS) $(OBJS) -o $(NAME)

all:	$(NAME)

clean:
		$(RM) $(OBJS)

fclean: clean
		rm -f $(NAME)

re:		fclean all

.PHONY:	all clean fclean lib bonus

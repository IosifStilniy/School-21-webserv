SRCS		=	Polls.cpp				\
				Server.cpp				\
				Meta.cpp				\
				Request.cpp				\
				RequestCollector.cpp	\
				Response.cpp			\
				Maintainer.cpp			\
				CGI.cpp					\
				utils.cpp				\
				webserv.cpp

OBJS		=	$(SRCS:.cpp=.o)

HEAD		=	./

NAME		=	webserv

GCC			=	c++

FLAGS		=	-Wall -Werror -Wextra -std=c++98

RM			=	rm -f

%.o:	%.cpp $(wildcard $(HEAD)/*.hpp)
		$(GCC) $(FLAGS) -c $< -o $@ 

$(NAME):	$(HEADS) $(OBJS)
			$(GCC) $(FLAGS) $(OBJS) -o $(NAME)

all:	$(NAME)

clean:
		$(RM) $(OBJS)

fclean: clean
		rm -f $(NAME)

re:		fclean all

.PHONY:	all clean fclean lib bonus

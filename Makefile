SRCS	:=	mini_serv.c
NAME	:=	mini_serv

all:
	@	gcc -g -Wall -Werror -Wextra $(SRCS) -o $(NAME)

rm:
	@	rm -rf $(NAME)

re: rm all

.PHONY: all rm re


SRCS_FILE := mini_serv.c
NAME :=	mini_serv

all:
	@ gcc -g -Wall -Werror -Wextra $(SRCS_FILE) -o $(NAME)


rm:
	@	rm -ff $(NAME)

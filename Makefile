NAME        := ipkcpd
SRCS        := main.c
OBJS        := main.o
CC          :=	gcc	 
CFLAGS      := -Wall -Wextra -Werror -pedantic -g -std=gnu99
RM          := rm -f
MAKEFLAGS   += --no-print-directory

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(OBJS) -o $(NAME)

clean:
	$(RM) $(OBJS)
	$(RM) vgcore.*

fclean: clean
	$(RM) $(NAME)

re:
	$(MAKE) fclean
	$(MAKE) all

tests: $(NAME)
	./$(NAME) tests

.PHONY: clean fclean re tests

NAME        := ipkcpd
SRCS        := main.c parser.c tcp_mode.c udp_mode.c tests.c
OBJS        := main.o parser.o tcp_mode.o udp_mode.o
CC          :=	gcc	 
CFLAGS      := -Wall -Wextra -pedantic -g -std=gnu99 
RM          := rm -f
MAKEFLAGS   += --no-print-directory

all: $(OBJS) 
	$(CC) $(CFLAGS) -o $(NAME) $^

clean:
	$(RM) $(OBJS)
	$(RM) vgcore.*
	$(RM) tests

fclean: clean
	$(RM) $(NAME)
	

re:
	$(MAKE) fclean
	$(MAKE) all


tests: $(SRCS)
	$(CC) $(CFLAGS) -DTESTING -o $@ $^
	./$@

.PHONY: clean fclean re 

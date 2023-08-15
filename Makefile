SRCDIR	:=	srcs
OBJDIR	:=	objs
INCDIR	:=	includes
FILES	:=	\
			main.c\
			ping_run.c\
			make_preference.c\
			option_parser.c\
			print_usage.c\
			host_address.c\
			socket.c\
			ping_session.c\
			ping_loop.c\
			ping_loop_send_ping.c\
			ping_loop_wait_pong.c\
			ping_sender.c\
			pong_receiver.c\
			protocol_ip.c\
			protocol_icmp.c\
			print_echo_reply.c\
			print_time_exceeded.c\
			print_ip_timestamp.c\
			analyze_received_datagram.c\
			stats.c\
			utils_math.c\
			utils_endian.c\
			utils_time.c\
			utils_error.c\
			utils_debug.c\

SRCS	:=	$(FILES:%.c=$(SRCDIR)/%.c)
OBJS	:=	$(FILES:%.c=$(OBJDIR)/%.o)
NAME	:=	ft_ping

LIBFT		:=	libft.a
LIBFT_DIR	:=	libft
CC			:=	gcc
CCOREFLAGS	=	-Wall -Wextra -Werror -O2 -I$(INCDIR) -I$(LIBFT_DIR)
CFLAGS		=	$(CCOREFLAGS) -D DEBUG -g -fsanitize=address -fsanitize=undefined
RM			:=	rm -rf

all:			$(NAME)


$(OBJDIR)/%.o:	$(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o:	%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(NAME):	$(OBJS) $(LIBFT)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS) $(LIBFT)

$(LIBFT):
	$(MAKE) -C $(LIBFT_DIR)
	cp $(LIBFT_DIR)/$(LIBFT) .

.PHONY:	clean
clean:
	$(RM) $(OBJDIR) $(LIBFT)

.PHONY:	fclean
fclean:			clean
	$(RM) $(LIBFT) $(NAME)
	$(MAKE) -C $(LIBFT_DIR) fclean

.PHONY:	re
re:				fclean all

.PHONY:	up
up:
	docker-compose up --build -d

.PHONY:	down
down:
	docker-compose down

.PHONY:	it
it:
	docker-compose exec app bash

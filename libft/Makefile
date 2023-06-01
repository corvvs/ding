NAME	:= libft.a
FUNCS	=	ft_strlen\
			ft_memset\
			ft_bzero\
			ft_memcmp\
			ft_memcpy\
			ft_memmove\
			ft_memchr\
			ft_isalpha\
			ft_isdigit\
			ft_isalnum\
			ft_isprint\
			ft_isascii\
			ft_toupper\
			ft_tolower\
			ft_strlcpy\
			ft_strlcat\
			ft_strchr\
			ft_strrchr\
			ft_strncmp\
			ft_strnstr\
			ft_atoi\
			ft_calloc\
			ft_strdup\
			ft_strmapi\
			ft_striteri\
			ft_putchar_fd\
			ft_putstr_fd\
			ft_putendl_fd\
			ft_putnbr_fd\
			ft_substr\
			ft_strjoin\
			ft_strtrim\
			ft_split\
			ft_itoa\
			ft_memutils

BFUNCS	=	ft_lstnew\
			ft_lstadd_front\
			ft_lstsize\
			ft_lstlast\
			ft_lstadd_back\
			ft_lstdelone\
			ft_lstclear\
			ft_lstiter\
			ft_lstmap

OBJS	= $(FUNCS:=.o)
BOBJS	= $(BFUNCS:=.o)

ifdef WITH_BONUS
OBJS += $(BOBJS)
endif

CC		:= cc
# CFLAGS	:= -Wall -Wextra -Werror -O2# -g -fsanitize=address
CFLAGS	:= -Wall -Wextra -Werror -O2 #-D GO_FAST #-g -fsanitize=address

all		: $(NAME)

$(NAME)	: $(OBJS)
	ar rcs $(NAME) $(OBJS)

clean	:
	$(RM) $(OBJS) $(BOBJS)

fclean	:	clean
	$(RM) $(NAME)

re		:	fclean all

bonus:
	$(MAKE) all WITH_BONUS=1

norm:
	python3 -m norminette *.{c,h}

.PHONY:	all clean fclean re bonus norm

CC					=gcc
CFLAGS				=-Wall -Wextra -Werror -g -D DEBUG
LDFLAGS				=-ltermcap
NAME				=ft_select
SRCDIR				=src/
SRC					=main.c
OBJDIR				=obj/
OBJ					=$(addprefix $(OBJDIR),$(SRC:.c=.o))
INCDIR				=inc/
INC					=$(addprefix $(INCDIR), select.h)
VPATH				=obj/

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJ): $(INC)

$(OBJDIR)%.o: $(SRCDIR)%.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -f $(OBJDIR)*.o

fclean: clean
	rm -f $(NAME)

re: fclean all

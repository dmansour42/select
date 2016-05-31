#include "../inc/select.h"


void reset_input_mode(struct termios *saved_attributes)
{
	static struct termios *s = NULL;

	if (s) {
		tcsetattr(STDIN_FILENO, TCSANOW, s);
		free(s);
	}
	else
		s = saved_attributes;
}

void fatal(char *err_message, void *whats_wrong) {
	whats_wrong ? fprintf(stderr, err_message, whats_wrong) : fputs(err_message, stderr);
	reset_input_mode(NULL);
	exit(EXIT_FAILURE);
}

void set_input_mode(void)
{
	struct termios tattr;
	struct termios *saved_attributes;

	if ((saved_attributes = (struct termios *)malloc(sizeof(struct termios))) == NULL)
	fatal("Not enought memory for malloc\n", NULL);
	/* Make sure stdin is a terminal. */
	if (!isatty(STDIN_FILENO))
		fatal("Not a terminal.\n", NULL);
	
	tcgetattr(STDIN_FILENO, saved_attributes);
	reset_input_mode(saved_attributes);

	/* Set the funny terminal modes. */
	tcgetattr(STDIN_FILENO, &tattr);
	tattr.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
	tattr.c_oflag &= ~OPOST;
	tattr.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
	tattr.c_cflag &= ~(CSIZE|PARENB);
	tattr.c_cflag |= CS8;
	tattr.c_cc[VMIN] = 1;
	tattr.c_cc[VTIME] = 0;
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &tattr);
}

void init_terminal_data()
{
	char *termtype = getenv("TERM");
	int success;

	if (termtype == 0)
		fatal("Specify a terminal type with `setenv TERM <yourtype>'.\n", NULL);

	success = tgetent(term_buffer, termtype);
	if (success < 0)
		fatal("Could not access the termcap data base.\n", NULL);
	if (success == 0)
		fatal("Terminal type `%s' is not defined.\n", termtype);
	printf("terminal tye is : %s\n", termtype);
}

void	push_node(t_list *l, char *data) {
	t_node *n;
	
	if ((n = (t_node *)malloc(sizeof(t_node))) == NULL)
		fatal("Not enought memory\n", NULL);
	n->data = data;
	l->size ? (n->current_position = 0) : (n->current_position = 1);
	if (l->size == 0) {
		l->current = n;
		n->next = n;
		n->prev = n;
	}
	else if (l->size == 1) {
		l->current->next = n;
		l->current->prev = n;
		n->next = l->current;
		n->prev = l->current;
	}
	else {
		l->current->prev->next = n;
		n->prev = l->current->prev;
		l->current->prev = n;
		n->next = l->current;
	}
	++l->size;
}

void	print_list(t_list *l, char *caps[USED]) {
		t_node *n;
		unsigned int s;
		int current_position;

		s = 0;
		n = l->current;
		while (s < l->size) {
			set_cursor_position(caps, 0, s);
			printf("%s\n", (char *)n->data);
			if (n->current_position)
				current_position = s;
			n = n->next;
			++s;
		}
		set_cursor_position(caps, 0, current_position);
}

t_list *make_list(int ac, char **av) {
	t_list *l;

	if ((l = (t_list *)malloc(sizeof(t_list))) == NULL) {
		fatal("Not enought memory available for malloc\n", NULL);
	}
	l->size = 0;
	l->current = NULL;
	while (ac > 1) {
		push_node(l, av[ac - 1]);
		ac--;
	}
	return (l);
}

int myputs(int n) {
	return (write(1, &n, 1));
}

void clearscreen(char *caps[USED]) {
	tputs(caps[CLEAR_SCREEN], 0, myputs);
}

void set_cursor_position(char *caps[USED], int pos_x, int pos_y) {
	tputs(tgoto(caps[POSITION_CURSOR], pos_x, pos_y), STDIN_FILENO, myputs);
}

void init_term_capabilities(char *caps[USED]) {
	int i;
	char *used_caps[USED];
	
	i = 0;
	used_caps[0] = "cl";
	used_caps[1] = "cm";
	while (i < USED) {
		if ((caps[i] = tgetstr(used_caps[i], NULL)) == 0)
			fatal("Missing termcap : %s\n", used_caps[i]);
		++i;
	}
}

int main(int ac, char **av) {
	t_list *l;
	char *caps[USED];

	if (ac == 1)
		return (EXIT_SUCCESS);
	set_input_mode();
	init_terminal_data();
	init_term_capabilities(caps);
	l = make_list(ac, av);

	clearscreen(caps);
	set_cursor_position(caps, 0, 0);
	print_list(l, caps);
	sleep(2);
	reset_input_mode(NULL);
	clearscreen(caps);
	return (EXIT_SUCCESS);
}

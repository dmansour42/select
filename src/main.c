#include "../inc/select.h"

t_list *getlist_singleton(t_list *l) {
	static t_list *ll = NULL;

	if (ll == NULL)
		ll = l;
	return (ll);
}

char **getcaps_singleton(char **caps) {
	static char **c = NULL;

	if (c == NULL)
		c = caps;
	return (c);
}

struct termios *termcap_singleton(struct termios *t) {
	static struct termios *tt = NULL;
	
	if (tt == NULL) {
		tt = t;
	}
	return (tt);
}

void	sigcont_handler(int s) {
	(void)s;
	set_input_mode(termcap_singleton(NULL));
	if (signal(SIGTSTP, sigtstp_handler) == SIG_ERR) {
		perror("signal :");
		fatal("Error while setting up signals handler\n.", NULL);
	}
	print_list(getlist_singleton(NULL));
}

void	sigtstp_handler(int s) {
	struct termios *t;
	char cmd[2];

	(void)s;
	t = termcap_singleton(NULL);
	reset_input_mode(NULL);
	if (signal(SIGTSTP, SIG_DFL) == SIG_ERR) {
		perror("signal :");
		fatal("Error while setting up signals handler\n.", NULL);
	}
	cmd[0] = t->c_cc[VSUSP];
	cmd[1] = 0;
	clearscreen();
	ioctl(0, TIOCSTI, cmd);
}

void	call_resizewindow(int s) {
	(void)s;
	resizewindow();
}

void	fatal_sig(int s) {
	clearscreen();
	printf("%d\n", s);
	fatal("Received signal to end program\n", NULL);
}

void handle_signals() {
	int i;

	i = 1;
	while (i < 32) {
		if (i == SIGWINCH) {
			if (signal(SIGWINCH, call_resizewindow) == SIG_ERR)
				fatal("Error while setting up signals handler ii\n.", NULL);
		}
		else if ((signal(i, fatal_sig) == SIG_ERR) && (i != SIGKILL) && (i != SIGSTOP)) {
			perror("signal :");
			fatal("Error while setting up signals handler\n.", NULL);
		}
		else if (i == SIGTSTP) {
			if (signal(SIGTSTP, sigtstp_handler) == SIG_ERR) {
				perror("signal :");
				fatal("Error while setting up signals handler\n.", NULL);
			}
		}
		else if (i == SIGCONT) {
			if (signal(SIGCONT, sigcont_handler) == SIG_ERR) {
				perror("signal :");
				fatal("Error while setting up signals handler\n.", NULL);
			}
		}
		++i;
	}
}


void reset_input_mode(struct termios *saved_attributes)
{
	static struct termios *s = NULL;

	if (s) {
		tcsetattr(STDIN_FILENO, TCSANOW, s);
	}
	else
		s = saved_attributes;
}

void fatal(char *err_message, void *whats_wrong) {
	whats_wrong ? fprintf(stderr, err_message, whats_wrong) : fputs(err_message, stderr);
	reset_input_mode(NULL);
	exit(EXIT_FAILURE);
}

void set_input_mode(struct termios *tattr)
{
	/* Set the funny terminal modes. */
	if (tcgetattr(STDIN_FILENO, tattr) == -1)
		perror("tcgetattr : ");
	//tattr.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
	//tattr.c_oflag &= ~OPOST;
	tattr->c_lflag &= ~(ICANON|ECHO);
	//tattr.c_cflag &= ~(CSIZE|PARENB);
	//tattr.c_cflag |= CS8;
	tattr->c_cc[VMIN] = 1;
	tattr->c_cc[VTIME] = 0;
	if (tcsetattr(STDIN_FILENO, TCSADRAIN, tattr) == -1)
		perror("tcsetattr : ");
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
	DLOG("terminal tye is : %s\n", termtype);
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

void	print_list(t_list *l) {
	t_node *n;
	unsigned int s;
	int current_position;

	s = 0;
	n = l->current;
	tputs(getcaps_singleton(NULL)[CLEAR_SCREEN], 0, myputs);
	while (s < l->size) {
		set_cursor_position(0, s);
		printf("%s\n", (char *)n->data);
		if (n->current_position)
			current_position = s;
		n = n->next;
		++s;
	}
	set_cursor_position(0, current_position);
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

void clearscreen(void) {
	tputs(getcaps_singleton(NULL)[CLEAR_SCREEN], 0, myputs);
}

void set_cursor_position(int pos_x, int pos_y) {
	char **caps = getcaps_singleton(NULL);

	tputs(tgoto(caps[POSITION_CURSOR], pos_x, pos_y), STDIN_FILENO, myputs);
}

void resizewindow(void) {
	static struct winsize w = {0, 0, 0, 0};

	ioctl(STDIN_FILENO, TIOCGWINSZ, &w);
	DLOG("Screen width: %i  Screen height: %i\n", w.ws_col, w.ws_row);
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
	getcaps_singleton(caps);
}

void	keyboard_hook(char c[CHAR_BUFSIZE]) {
	write(1, c, 1);
}

int main(int ac, char **av) {
	t_list *l;
	char *caps[USED];
	char c[CHAR_BUFSIZE];
	struct termios saved_attributes;
	struct termios tattr;

	if (ac == 1)
		return (EXIT_SUCCESS);
	if (!isatty(STDIN_FILENO))
		fatal("Not a terminal.\n", NULL);

	tcgetattr(STDIN_FILENO, &saved_attributes);
	reset_input_mode(&saved_attributes);

	init_terminal_data();
	set_input_mode(&tattr);
	termcap_singleton(&tattr);

	init_term_capabilities(caps);

	l = make_list(ac, av);
	getlist_singleton(l);

	print_list(l);

	handle_signals();

	while (1)
	{
		read(STDIN_FILENO, c, 1);
		keyboard_hook(c);
	}

	reset_input_mode(NULL);
	clearscreen();
	return (EXIT_SUCCESS);
}

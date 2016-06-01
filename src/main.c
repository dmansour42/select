#include "../inc/select.h"

void	sigcnt_handler(int s) {
	(void)s;
	set_input_mode();
}
void	sigstp_handler(int s) {
	(void)s;
	reset_input_mode(NULL);
	//signal(SIGSTOP, SIG_DFL);
}

void	call_resizewindow(int s) {
	(void)s;
	resizewindow();
}

void	fatal_sig(int s) {
	printf("%d\n", s);
	fatal("Received signal to end program\n", NULL);
}

void handle_signals() {
	int i;

	i = 1;
	while (i < 32) {
		if (i == SIGTSTP)
			signal(i, sigstp_handler);
		else if (i == SIGCONT)
			signal(i, sigcnt_handler);
		else if (i == SIGWINCH) {
			if (signal(SIGWINCH, call_resizewindow) == SIG_ERR)
				fatal("Error while setting up signals handler ii\n.", NULL);
		}
		else if ((signal(i, fatal_sig) == SIG_ERR) && (i != SIGKILL) && (i != SIGSTOP)) {
			perror("signal :");
			fatal("Error while setting up signals handler\n.", NULL);
		}
		else if (i == SIGTTIN)
			if (signal(i, sigcnt_handler) == SIG_ERR)
				fatal("Error while setting up signals handler ii\n.", NULL);
		++i;
	}
}


void reset_input_mode(struct termios *saved_attributes)
{
	static struct termios *s = NULL;

	if (s) {
		tcsetattr(STDIN_FILENO, TCSANOW, s);
	//	free(s);
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
	if (tcgetattr(STDIN_FILENO, &tattr) == -1)
		perror("tcgetattr : ");
	//tattr.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
	//tattr.c_oflag &= ~OPOST;
	tattr.c_lflag &= ~(ICANON|ECHO);
	//tattr.c_cflag &= ~(CSIZE|PARENB);
	//tattr.c_cflag |= CS8;
	tattr.c_cc[VMIN] = 1;
	tattr.c_cc[VTIME] = 0;
	if (tcsetattr(STDIN_FILENO, TCSADRAIN, &tattr) == -1)
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
}

void	keyboard_hook(char c[CHAR_BUFSIZE]) {
		write(1, c, 1);
}

int main(int ac, char **av) {
	t_list *l;
	char *caps[USED];
	char c[CHAR_BUFSIZE];

	if (ac == 1)
		return (EXIT_SUCCESS);
	set_input_mode();
	init_terminal_data();
	init_term_capabilities(caps);
	l = make_list(ac, av);

	clearscreen(caps);
	set_cursor_position(caps, 0, 0);
	print_list(l, caps);

	handle_signals();

	tputs(caps[CLEAR_SCREEN], 0, myputs);


	while (1)
	{
		read(STDIN_FILENO, c, 1);
		keyboard_hook(c);
	}

	reset_input_mode(NULL);
	clearscreen(caps);
	return (EXIT_SUCCESS);
}

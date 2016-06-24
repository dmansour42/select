#include "../inc/select.h"

t_keyhooks *get_keyhookssingleton(t_keyhooks *k) {
	static t_keyhooks *kk = NULL;

	if (kk == NULL)
		kk = k;
	return (kk);
}

void	windows_to_small(void) {
	struct winsize *w;
	static int i = 1;
	static char *msg = "Windows is too small...";
	int len_to_print;
	
	len_to_print = 22 - (i % 22);
	w = get_window_sizesingleton(NULL);
	clearscreen();
	set_cursor_position(0, 0);
	if (w->ws_col > len_to_print)
		write(1, msg + (i % 22), len_to_print);
	else
		write(1, msg + (i % 22), w->ws_col);
	++i;
	set_cursor_position(0, 0);
}

int		get_current_positionsingleton(int p) {
	static int pp = 0;
	t_list *l;

	l = getlist_singleton(NULL);
	pp += p;
	if (pp < 0)
		pp = l->size - 1;
	else if (pp == l->size)
		pp = 0;
	return (pp);
}

t_list *getlist_singleton(t_list *l) {
	static t_list *ll = NULL;

	if (ll == NULL)
		ll = l;
	return (ll);
}

char **getcaps_singleton(void) {
	static char *c[USED] = {NULL};

	if (c[0] == NULL) {
		init_term_capabilities(c);
	}
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
	print_list();
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
	fprintf(stderr, "%d\n", s);
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
}

void	push_node(t_list *l, char *data) {
	t_node *n;

	if ((n = (t_node *)malloc(sizeof(t_node))) == NULL)
		fatal("Nowt enought memory\n", NULL);
	n->data = data;
	n->len = strlen(data);
	n->selected = FALSE;
	n->position = 0;
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

void	delete_node(void) {
	t_list *l;
	t_node *n;

	l = getlist_singleton(NULL);
	n = get_current_node();
	if (l->size == 1) {
		fatal("No more elements in the list\n", NULL);
	}

	n->prev->next = n->next;
	n->next->prev = n->prev;
	n->next->current_position = TRUE;
	if (l->current == n)
		l->current = n->next;
	free(n);
	n = NULL;
	--l->size;
}

void print_element(struct winsize *w, t_node *n) {
	int len_to_print;
	
	len_to_print = n->len - (n->position % n->len);
	w = get_window_sizesingleton(NULL);
	if (n->len <= w->ws_col) {
		n->position = 0;
		write(1, n->data, n->len);
		return;
	}
	if (w->ws_col > len_to_print)
		write(1, n->data + (n->position % n->len), len_to_print);
	else
		write(1, n->data, w->ws_col);
}

void	print_list() {
	t_list *l;
	t_node *n;
	int s;
	int current_position;
	struct winsize *w;
	int toskip;

	l = getlist_singleton(NULL);
	n = l->current;
	s = 0;
	current_position = get_current_positionsingleton(0);
	w = get_window_sizesingleton(NULL);
	set_cursor_position(0, 0);
	tputs(getcaps_singleton()[CLEAR_SCREEN], 0, myputs);
	if (w->ws_row <= 0) {
		write(1, "Windows is too small...", 22);
		return ;
	}
	toskip = current_position / (w->ws_row) * (w->ws_row);

	if (l->size - 1 <= w->ws_row) {
		while (s < l->size) {
			set_cursor_position(0, s);
			if (n->selected == TRUE) {
				highlight(); print_element(w, n); highlight();
				}
			else
				print_element(w, n);
			n = n->next;
			++s;
		}
		set_cursor_position(0, current_position);
		return ;
	}


	while (s < toskip) {
		++s;
		n = n->next;
	}
	while (s < l->size) {
	// MODIF DE == a >
		if (s - toskip > w->ws_row && l->size > 1)
		{
			break ;
		}
		set_cursor_position(0, s - toskip);
		if (n->selected == TRUE)
		{
			highlight(); print_element(w, n); highlight();
			}
		else
			print_element(w, n);
		n = n->next;
		++s;
	}
	if (s < l->size) {
		set_cursor_position(0, (w->ws_row));
		write(1, "...", 3);
		set_cursor_position(0, current_position % (w->ws_row));
	}
	else {
		set_cursor_position(0, current_position % (w->ws_row));
	}
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
	tputs(getcaps_singleton()[CLEAR_SCREEN], 0, myputs);
}

void set_cursor_position(int pos_x, int pos_y) {
	char **caps = getcaps_singleton();

	tputs(tgoto(caps[POSITION_CURSOR], pos_x, pos_y), STDIN_FILENO, myputs);
}

void resizewindow(void) {
	static int action_flag = 0;
	static struct winsize w = {0, 0, 0, 0};
	static t_list *l = NULL;
	t_keyhooks *k;
	int i;

	i = 0;
	k = get_keyhookssingleton(NULL);
	ioctl(STDIN_FILENO, TIOCGWINSZ, &w);
	get_window_sizesingleton(&w);
	--w.ws_row;
	if (w.ws_row <= 0) {
		k[PRINT].action = windows_to_small;
		while (i < ESC) {
			k[i].action = do_nothing;
			action_flag = FALSE;
			++i;
		}
	}
	else if (action_flag == FALSE) {
		reinit_keys(k);
		action_flag = TRUE;
	}
	if (l == NULL)
		l = getlist_singleton(NULL);
	print_list();
}

struct winsize *get_window_sizesingleton(struct winsize *w) {
	static struct winsize *ww = NULL;

	if (ww == NULL) {
		ww = w;
	}
	return (ww);
}

void highlight(void) {
	char **caps = getcaps_singleton();
	static int s = TRUE;

	s == TRUE ? (s = FALSE) : (s = TRUE);
	if (s == FALSE)
		tputs(caps[HIGHLIGHT_ON], 0, myputs);
	else
		tputs(caps[HIGHLIGHT_OFF], 0, myputs);
}

t_node *get_current_node(void) {
	t_list	*l;
	t_node	*n;

	l = getlist_singleton(NULL);
	n = l->current;
	while (n->current_position == FALSE) {
		n = n->next;
	}
	return (n);
}

void init_term_capabilities(char *caps[USED]) {
	int i;
	char *used_caps[USED];

	i = 0;
	used_caps[CLEAR_SCREEN] = "cl";
	used_caps[POSITION_CURSOR] = "cm";
	used_caps[HIGHLIGHT_ON] = "so";
	used_caps[HIGHLIGHT_OFF] = "se";
	while (i < USED) {
		if ((caps[i] = tgetstr(used_caps[i], NULL)) == NULL)
			fatal("Missing termcap : %s\n", used_caps[i]);
		++i;
	}
}

void	do_nothing(void) {
	return ;
}

void	uparrow_pressed(void) {
	t_node *n = get_current_node();

	n = get_current_node();
	n->current_position = FALSE;
	n->prev->current_position = TRUE;
	get_current_positionsingleton(-1);
}

void	downarrow_pressed(void) {
	t_node *n;

	n = get_current_node();
	n->current_position = FALSE;
	n->next->current_position = TRUE;
	get_current_positionsingleton(1);
}

void	leftarrow_pressed(void) {
	t_node *n;

	n = get_current_node();
	--n->position;
	if (n->position == -1)
		n->position = n->len - 1;
	//	printf("left arrow pressed\n");
}

void	rightarrow_pressed(void) {
	t_node *n;

	n = get_current_node();
	++n->position;
	if (n->position == n->len)
		n->position = 0;
	//	printf("right arrow pressed\n");
}

void	enter_pressed(void) {
	t_node *n = get_current_node();

	n = get_current_node();
	n->selected == TRUE ? (n->selected = FALSE) : (n->selected = TRUE);
}

void	esc_pressed(void) {
	t_list *l;
	t_node *n;
	int i;

	l = getlist_singleton(NULL);
	n = l->current;
	i = l->size;
	clearscreen();
	while (i) {
		if (l->current->selected == TRUE)
			printf("%s ", l->current->data);
		if (n->next)
			n = n->next;
		l->current = NULL;
		free(l->current);
		l->current = n;
		--i;
	}
	free(l);
	printf("\n");
	reset_input_mode(NULL);
	exit(0);
}

void	del_pressed(void) {
	delete_node();
}

void	reinit_keys(t_keyhooks *k) {
	k[UPARROW].action = uparrow_pressed;
	k[DOWNARROW].action = downarrow_pressed;
	k[LEFTARROW].action = leftarrow_pressed;
	k[RIGHTARROW].action = rightarrow_pressed;
	k[ENTER].action = enter_pressed;
	k[DEL].action = del_pressed;
	k[ESC].action = esc_pressed;
	k[PRINT].action = print_list;
}

void	init_keys(t_keyhooks t[KEYS_USED]) {
	strcpy(t[UPARROW].s, UPARROW_CODE);
	strcpy(t[DOWNARROW].s, DOWNARROW_CODE);
	strcpy(t[LEFTARROW].s, LEFTARROW_CODE);
	strcpy(t[RIGHTARROW].s, RIGHTARROW_CODE);
	strcpy(t[ENTER].s, ENTER_CODE);
	strcpy(t[DEL].s, DEL_CODE);
	strcpy(t[ESC].s, ESC_CODE);
	t[UPARROW].action = uparrow_pressed;
	t[DOWNARROW].action = downarrow_pressed;
	t[LEFTARROW].action = leftarrow_pressed;
	t[RIGHTARROW].action = rightarrow_pressed;
	t[ENTER].action = enter_pressed;
	t[DEL].action = del_pressed;
	t[ESC].action = esc_pressed;
	t[PRINT].action = print_list;
}

void	keyboard_hook(char c[CHAR_BUFSIZE], t_keyhooks k[KEYS_USED]) {
	int i;

	i = 0;
	while (i < KEYS_USED) {
		if (strncmp(k[i].s, c, CHAR_BUFSIZE) == 0) {
			k[i].action();
			k[PRINT].action();
			return ;
		}
		++i;
	}
}

int main(int ac, char **av) {
	t_list *l;
	char *caps[USED];
	t_keyhooks keys[KEYS_USED];
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
	init_keys(keys);
	get_keyhookssingleton(keys);

	l = make_list(ac, av);
	getlist_singleton(l);
	resizewindow();
	//print_list();

	handle_signals();
	
	bzero(c, CHAR_BUFSIZE);

	while (read(STDIN_FILENO, c, 4) != -1)
	{
		keyboard_hook(c, keys);
		bzero(c, CHAR_BUFSIZE);
	}

	reset_input_mode(NULL);
	clearscreen();
	return (EXIT_SUCCESS);
}

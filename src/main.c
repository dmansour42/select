#include "../inc/select.h"

int		get_current_positionsingleton(int p) {
	static int pp = 0;
	t_list *l;

	l = getlist_singleton(NULL);
	if (p >= 0)
		pp = (p % l->size);
	return (pp);
}

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
		fatal("Nowt enought memory\n", NULL);
	n->data = data;
	n->selected = FALSE;
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
	int s;
	int current_position;
	struct winsize *w;

	n = l->current;
	s = 0;
	current_position = get_current_positionsingleton(-1);
	w = get_window_sizesingleton(NULL);
	set_cursor_position(0, 0);
	tputs(getcaps_singleton(NULL)[CLEAR_SCREEN], 0, myputs);


	// why ws_row - 2 ? because lat line of window 
	//if not enought space to print he whole list is a '...'
	// and just after, an empty line where we cant write.
	/*
	if (w->ws_row <= 1 && l->size > 1) {
		printf("Windows is too small...\n");
		return ;
	}
*/
	DLOG("row == %d\n", w->ws_row);
	if (w->ws_row > 2) {
		while (s < (current_position / w->ws_row - 2) * (w->ws_row - 2)) {
			++s;
			n = n->next;
		}
	}
	while (s < l->size) {
		if (s == w->ws_row - 2 && l->size > 1)
			break ;
		set_cursor_position(0, s);
		if (n->selected == TRUE)
			highlight(), printf("%s\n", n->data), highlight();
		else
			printf("%s\n", n->data);
		n = n->next;
		++s;
	}
	if (s == 0) {
		printf("Windows is too small...\n");
	}
	else if (s < l->size) {
		DLOG("rows == %d\n", s);
		set_cursor_position(0, s);
		printf("...\n");
		set_cursor_position(0, current_position % (w->ws_row - 2));
	}
	else
		set_cursor_position(0, current_position % (w->ws_row));
}
/*
   void	print_list(t_list *l) {
   t_node *n;
   int s;
   int current_position;
   struct winsize *w;

   s = 0;
   current_position = get_current_positionsingleton(-1);
   n = l->current;
   w = get_window_sizesingleton(NULL);
   tputs(getcaps_singleton(NULL)[CLEAR_SCREEN], 0, myputs);
   if (w->ws_row != 0) {
   while (s < (current_position / w->ws_row) * w->ws_row) { //avoid % 0 !
   DLOG("calculated offset %d\n", current_position / w->ws_row * w->ws_row);
   n = n->next;
   ++s;
   }
   }
   while (s < l->size) {
   set_cursor_position(0, s);
   if (s == (w->ws_row - 2)) {
   DLOG("rows == %d\n", w->ws_row);
   printf("...\n");
   break ;
   }
   if (n->selected == TRUE)
   highlight(), printf("%s\n", n->data), highlight();
   else
   printf("%s\n", n->data);
//		if (n->current_position)
//			current_position = s;
n = n->next;
++s;
}
if (w->ws_row != 0 && current_position != 0)
{
set_cursor_position(0, (current_position % w->ws_row));
DLOG("%d\n", (current_position % w->ws_row));
}
else
set_cursor_position(0, 0);
}
*/

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
	static t_list *l = NULL;

	ioctl(STDIN_FILENO, TIOCGWINSZ, &w);
	get_window_sizesingleton(&w);
	if (l == NULL)
		l = getlist_singleton(NULL);
	print_list(l);
}

struct winsize *get_window_sizesingleton(struct winsize *w) {
	static struct winsize *ww = NULL;

	if (ww == NULL)
		ww = w;
	return (ww);
}

void highlight(void) {
	char **caps = getcaps_singleton(NULL);
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
		if ((caps[i] = tgetstr(used_caps[i], NULL)) == 0)
			fatal("Missing termcap : %s\n", used_caps[i]);
		++i;
	}
	getcaps_singleton(caps);
}

void	uparrow_pressed(void) {
	t_node *n = get_current_node();

	n = get_current_node();
	n->current_position = FALSE;
	n->prev->current_position = TRUE;
}

void	downarrow_pressed(void) {
	struct winsize *w;
	t_node *n;
	int p;

	n = get_current_node();
	w = get_window_sizesingleton(NULL);
	n->current_position = FALSE;
	n->next->current_position = TRUE;
	p = get_current_positionsingleton(-1);
	get_current_positionsingleton(p + 1);
}

void	leftarrow_pressed(void) {
	//	printf("left arrow pressed\n");
}

void	rightarrow_pressed(void) {
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

void	init_keys(t_keyhooks t[KEYS_USED]) {
	strcpy(t[UPARROW].s, UPARROW_CODE);
	strcpy(t[DOWNARROW].s, DOWNARROW_CODE);
	strcpy(t[LEFTARROW].s, LEFTARROW_CODE);
	strcpy(t[RIGHTARROW].s, RIGHTARROW_CODE);
	strcpy(t[ENTER].s, ENTER_CODE);
	strcpy(t[ESC].s, ESC_CODE);
	t[UPARROW].action = uparrow_pressed;
	t[DOWNARROW].action = downarrow_pressed;
	t[LEFTARROW].action = leftarrow_pressed;
	t[RIGHTARROW].action = rightarrow_pressed;
	t[ENTER].action = enter_pressed;
	t[ESC].action = esc_pressed;
}

void	keyboard_hook(char c[CHAR_BUFSIZE], t_keyhooks k[KEYS_USED]) {
	int i;

	i= 0;
	while (i < KEYS_USED) {
		if (strncmp(k[i].s, c, CHAR_BUFSIZE) == 0) {
			k[i].action();
			print_list(getlist_singleton(NULL));
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

	l = make_list(ac, av);
	getlist_singleton(l);
	resizewindow();

	handle_signals();

	while (1)
	{
		bzero(c, CHAR_BUFSIZE);
		read(STDIN_FILENO, c, 4);
		keyboard_hook(c, keys);
	}

	reset_input_mode(NULL);
	clearscreen();
	return (EXIT_SUCCESS);
}

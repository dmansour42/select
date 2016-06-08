#ifndef SELECT_H
#define SELECT_H

# include <stdio.h>
# include <unistd.h>
# include <string.h>
# include <sys/types.h>
# include <sys/ioctl.h>
# include <stdlib.h>
# include <errno.h>
# include <stdarg.h>
# include <term.h>
# include <signal.h>
# include <errno.h>

# define SIGNAL_USED 31
# define CHAR_BUFSIZE 4
# define TRUE 1
# define FALSE 0
# define UPARROW_CODE "\33\133\101\0"
# define DOWNARROW_CODE "\33\133\102\0"
# define RIGHTARROW_CODE "\33\133\103\0"
# define LEFTARROW_CODE "\33\133\104\0"
# define ENTER_CODE "\12\0\0\0"
# define ESC_CODE "\33\0\0\0"

# ifdef DEBUG
#  define DLOG(...) fprintf (stderr, __VA_ARGS__ )
# else
#  define DLOG(...) 
# endif

# ifdef unix
   static char term_buffer[2048];
# else
#  define term_buffer 0
# endif

enum {
	CLEAR_SCREEN,
	POSITION_CURSOR,
	HIGHLIGHT_ON,
	HIGHLIGHT_OFF,
	USED
};

enum {
	UPARROW,
	DOWNARROW,
	LEFTARROW,
	RIGHTARROW,
	ENTER,
	ESC,
	KEYS_USED
};

typedef struct 				s_keyhooks {
		char				s[5];
		void				(*action)(void);
}							t_keyhooks;

typedef struct				s_node {
		char			*data;
		struct s_node	*next;
		struct s_node	*prev;
		int				current_position;
		int				selected;
}							t_node;	


typedef struct				s_list {
		unsigned int size;
		t_node		*current;
}							t_list;	

void set_cursor_position(int pos_x, int pos_y);
void resizewindow(void);
void fatal(char *err_message, void *whats_wrong);
void reset_input_mode(struct termios *saved_attributes);
void set_input_mode(struct termios *tattr);
void sigtstp_handler(int s);
int myputs(int n);
void print_list(t_list *l);
void clearscreen(void);
void highlight(void);

struct termios* termcaps_singleton(struct termios*t);




typedef struct				s_terminfos {
	char				pc;
	short				ospeed;
}							t_terminfos;


#endif

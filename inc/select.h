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
	USED
};

typedef struct				s_node {
		char			*data;
		struct s_node	*next;
		struct s_node	*prev;
		int				current_position;
}							t_node;	


typedef struct				s_list {
		unsigned int size;
		t_node		*current;
}							t_list;	

void set_cursor_position(char *caps[USED], int pos_x, int pos_y);
void resizewindow(void);
void fatal(char *err_message, void *whats_wrong);
void reset_input_mode(struct termios *saved_attributes);
void set_input_mode(void);





typedef struct				s_terminfos {
	char				pc;
	short				ospeed;
}							t_terminfos;


#endif

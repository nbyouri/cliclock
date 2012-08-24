/* cli-clock 
 * See line 72 to set foreground color. 
 * Thanks to xorg-62 for cli-clock
*/
#include <stdlib.h>
#include <time.h>
#include <ncurses.h>
#include <signal.h>

typedef enum { False, True } Bool;

/* Global cliclock struct */
typedef struct
{
     /* while() boolean */
     Bool running;
     int bg;

		 struct
		 {
				 int color;
				 long delay;
		 } option;

     /* Clock geometry */
     struct
     {
          int x, y, w, h;
     } geo;

     /* Date content ([2] = number by number) */
     struct
     {
          unsigned int hour[2];
          unsigned int minute[2];
          unsigned int second[2];
     } date;

     /* time.h utils */
     struct tm *tm;
     time_t lt;

     /* Clock member */
     WINDOW *framewin;

} cliclock_t;

/* Prototypes */
void init(void);
void update_hour(void);
void draw_number(int n, int x, int y);
void draw_clock(void);
void clock_move(int x, int y);
void set_win(void);
void signal_handler(int signal);

/* Global variable */
cliclock_t *cliclock;

/* Number matrix */
const Bool number[][15] =
{
     {1,1,1,1,0,1,1,0,1,1,0,1,1,1,1}, /* 0 */
     {0,0,1,0,0,1,0,0,1,0,0,1,0,0,1}, /* 1 */
     {1,1,1,0,0,1,1,1,1,1,0,0,1,1,1}, /* 2 */
     {1,1,1,0,0,1,1,1,1,0,0,1,1,1,1}, /* 3 */
     {1,0,1,1,0,1,1,1,1,0,0,1,0,0,1}, /* 4 */
     {1,1,1,1,0,0,1,1,1,0,0,1,1,1,1}, /* 5 */
     {1,1,1,1,0,0,1,1,1,1,0,1,1,1,1}, /* 6 */
     {1,1,1,0,0,1,0,0,1,0,0,1,0,0,1}, /* 7 */
     {1,1,1,1,0,1,1,1,1,1,0,1,1,1,1}, /* 8 */
     {1,1,1,1,0,1,1,1,1,0,0,1,1,1,1}, /* 9 */
};
/* User changeable options. */
/* Available:
 * COLOR_BLACK
 * COLOR_RED
 * COLOR_GREEN
 * COLOR_YELLOW
 * COLOR_BLUE
 * COLOR_MAGENTA
 * COLOR_CYAN
 * COLOR_WHITE
*/
#define FG_COLOR COLOR_BLUE;

void
init(void)
{
     struct sigaction sig;
		 cliclock->bg = 0;

     /* Init ncurses */
     initscr();
     cbreak();
     noecho();
     keypad(stdscr, True);
     start_color();
     curs_set(False);
     clear();

		 if(use_default_colors() == OK)
     cliclock->bg = -1;

     /* Init color pair */
     init_pair(0, cliclock->bg, cliclock->bg);
     init_pair(1, cliclock->bg, cliclock->option.color);
     init_pair(2, cliclock->option.color, cliclock->bg);
		 
     refresh();
    
		 /* Init signal handler */
		 sig.sa_handler = signal_handler;
		 sig.sa_flags   = 0;
		 sigaction(SIGWINCH, &sig, NULL);

     /* Init global struct */ 
     cliclock->running = True;
     if(!cliclock->geo.x)
          cliclock->geo.x = 0;
     if(!cliclock->geo.y)
          cliclock->geo.y = 0;
     cliclock->geo.w = 54;
     cliclock->geo.h = 7;
     cliclock->tm = localtime(&(cliclock->lt));
     cliclock->lt = time(NULL);
     update_hour();

     /* Create clock win */ 
     cliclock->framewin = newwin(cliclock->geo.h,
                                 cliclock->geo.w,
                                 cliclock->geo.x,
                                 cliclock->geo.y);
                      clock_move((LINES / 2 - (cliclock->geo.h / 2)),
                                 (COLS  / 2 - (cliclock->geo.w / 2)));
                                 wborder(cliclock->framewin, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
		 nodelay(stdscr, True);
     wrefresh(cliclock->framewin);

     return;
}
void
signal_handler(int signal)
{
		switch(signal)
		{
				case SIGWINCH:
						endwin();
						init();
						break;
		}
		return;
}
void
update_hour(void)
{
     int ihour;

     cliclock->tm = localtime(&(cliclock->lt));
     cliclock->lt = time(NULL);

     ihour = cliclock->tm->tm_hour;

     /* Set hour */
     cliclock->date.hour[0] = ihour / 10;
     cliclock->date.hour[1] = ihour % 10;

     /* Set minutes */
     cliclock->date.minute[0] = cliclock->tm->tm_min / 10;
     cliclock->date.minute[1] = cliclock->tm->tm_min % 10;

     /* Set seconds */
     cliclock->date.second[0] = cliclock->tm->tm_sec / 10;
     cliclock->date.second[1] = cliclock->tm->tm_sec % 10;

     return;
}
void
draw_number(int n, int x, int y)
{
     int i, sy = y;

     for(i = 0; i < 30; ++i, ++sy)
     {
          if(sy == y + 6)
          {
               sy = y;
               ++x;
          }
          wbkgdset(cliclock->framewin, COLOR_PAIR(number[n][i/2]));
          mvwaddch(cliclock->framewin, x, sy, ' ');
     }
     wrefresh(cliclock->framewin);

     return;
}
void
draw_clock(void)
{
     /* Draw hour numbers */
     draw_number(cliclock->date.hour[0], 1, 1);
     draw_number(cliclock->date.hour[1], 1, 8);

     /* 2 dot for number separation */
     wbkgdset(cliclock->framewin, COLOR_PAIR(1));
     mvwaddstr(cliclock->framewin, 2, 16, "  ");
     mvwaddstr(cliclock->framewin, 4, 16, "  ");

     /* Draw minute numbers */
     draw_number(cliclock->date.minute[0], 1, 20);
     draw_number(cliclock->date.minute[1], 1, 27);

     /* 2 dot for number separation */
     wbkgdset(cliclock->framewin, COLOR_PAIR(1));
     mvwaddstr(cliclock->framewin, 2, 35, "  ");
     mvwaddstr(cliclock->framewin, 4, 35, "  ");

     /* Draw second numbers */
     draw_number(cliclock->date.second[0], 1, 39);
     draw_number(cliclock->date.second[1], 1, 46);
}
void
clock_move(int x, int y)
{
     mvwin(cliclock->framewin, (cliclock->geo.x = x), (cliclock->geo.y = y));
     return;
}
void 
key_event(void)
{
		int c;
    struct timespec length = { 0, cliclock->option.delay };
		switch(c = wgetch(stdscr))
		{
				case 'q':
						cliclock->running = False;
						break;
				default:
						nanosleep(&length, NULL);
						break;
		}
		return;
}

int
main(void)
{
		 cliclock = malloc(sizeof(cliclock_t));
     cliclock->option.color = FG_COLOR; 
     cliclock->option.delay = 1600000; /* 1FPS */
     init();
     while(cliclock->running)
     {
          update_hour();
          draw_clock();
					key_event();
     }
     free(cliclock);
     endwin();
     return 0;
}

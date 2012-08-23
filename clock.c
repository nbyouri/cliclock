#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <ncurses.h>
#include <unistd.h>
#include <getopt.h>

/* Macro */
#define NORMFRAMEW 35
#define SECFRAMEW  54

typedef enum { False, True } Bool;

/* Global ttyclock struct */
typedef struct
{
     /* while() boolean */
     Bool running;
     int bg;

     /* Running option */
     struct
     {
          int color;
     } option;

     /* Clock geometry */
     struct
     {
          int x, y, w, h;
          int a, b;
     } geo;

     /* Date content ([2] = number by number) */
     struct
     {
          unsigned int hour[2];
          unsigned int minute[2];
          unsigned int second[2];
          char datestr[256];
     } date;

     /* time.h utils */
     struct tm *tm;
     time_t lt;

     /* Clock member */
     WINDOW *framewin;

} ttyclock_t;

/* Prototypes */
void init(void);
void signal_handler(int signal);
void update_hour(void);
void draw_number(int n, int x, int y);
void draw_clock(void);
void clock_move(int x, int y, int w, int h);
void set_second(void);
void set_center(void);

/* Global variable */
ttyclock_t *ttyclock;

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

void
init(void)
{
     struct sigaction sig;
     ttyclock->bg = COLOR_BLACK;

     /* Init ncurses */
     initscr();
     cbreak();
     noecho();
     keypad(stdscr, True);
     start_color();
     curs_set(False);
     clear();

     /* Init default terminal color */
     if(use_default_colors() == OK)
          ttyclock->bg = -1;

     /* Init color pair */
     init_pair(0, ttyclock->bg, ttyclock->bg);
     init_pair(1, ttyclock->bg, ttyclock->option.color);
     init_pair(2, ttyclock->option.color, ttyclock->bg);
     refresh();

     /* Init signal handler */
     sig.sa_handler = signal_handler;
     sig.sa_flags   = 0;
     sigaction(SIGWINCH, &sig, NULL);
     sigaction(SIGTERM,  &sig, NULL);
     sigaction(SIGINT,   &sig, NULL);
     sigaction(SIGSEGV,  &sig, NULL);

     /* Init global struct */
     ttyclock->running = True;
     if(!ttyclock->geo.x)
          ttyclock->geo.x = 0;
     if(!ttyclock->geo.y)
          ttyclock->geo.y = 0;
     if(!ttyclock->geo.a)
          ttyclock->geo.a = 1;
     if(!ttyclock->geo.b)
          ttyclock->geo.b = 1;
     ttyclock->geo.w = SECFRAMEW;
     ttyclock->geo.h = 7;
     ttyclock->tm = localtime(&(ttyclock->lt));
     ttyclock->lt = time(NULL);
     update_hour();

     /* Create clock win */
     ttyclock->framewin = newwin(ttyclock->geo.h,
                                 ttyclock->geo.w,
                                 ttyclock->geo.x,
                                 ttyclock->geo.y);
     box(ttyclock->framewin, 0, 0);
     wborder(ttyclock->framewin, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');

     set_center();

     wrefresh(ttyclock->framewin);

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
          /* Interruption signal */
     case SIGINT:
     case SIGTERM:
          ttyclock->running = False;
          /* Segmentation fault signal */
          break;
     case SIGSEGV:
          endwin();
          fprintf(stderr, "Segmentation fault.\n");
          exit(EXIT_FAILURE);
          break;
     }

     return;
}

void
update_hour(void)
{
     int ihour;

     ttyclock->tm = localtime(&(ttyclock->lt));
     ttyclock->lt = time(NULL);

     ihour = ttyclock->tm->tm_hour;

     /* Set hour */
     ttyclock->date.hour[0] = ihour / 10;
     ttyclock->date.hour[1] = ihour % 10;

     /* Set minutes */
     ttyclock->date.minute[0] = ttyclock->tm->tm_min / 10;
     ttyclock->date.minute[1] = ttyclock->tm->tm_min % 10;

     /* Set seconds */
     ttyclock->date.second[0] = ttyclock->tm->tm_sec / 10;
     ttyclock->date.second[1] = ttyclock->tm->tm_sec % 10;

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
          wbkgdset(ttyclock->framewin, COLOR_PAIR(number[n][i/2]));
          mvwaddch(ttyclock->framewin, x, sy, ' ');
     }
     wrefresh(ttyclock->framewin);

     return;
}

void
draw_clock(void)
{
     /* Draw hour numbers */
     draw_number(ttyclock->date.hour[0], 1, 1);
     draw_number(ttyclock->date.hour[1], 1, 8);

     /* 2 dot for number separation */
     wbkgdset(ttyclock->framewin, COLOR_PAIR(1));
     mvwaddstr(ttyclock->framewin, 2, 16, "  ");
     mvwaddstr(ttyclock->framewin, 4, 16, "  ");

     /* Draw minute numbers */
     draw_number(ttyclock->date.minute[0], 1, 20);
     draw_number(ttyclock->date.minute[1], 1, 27);

     /* 2 dot for number separation */
     wbkgdset(ttyclock->framewin, COLOR_PAIR(1));
     mvwaddstr(ttyclock->framewin, 2, NORMFRAMEW, "  ");
     mvwaddstr(ttyclock->framewin, 4, NORMFRAMEW, "  ");

     /* Draw second numbers */
     draw_number(ttyclock->date.second[0], 1, 39);
     draw_number(ttyclock->date.second[1], 1, 46);
}

void
clock_move(int x, int y, int w, int h)
{

     /* Erase border for a clean move */
     wbkgdset(ttyclock->framewin, COLOR_PAIR(0));
     wborder(ttyclock->framewin, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
     werase(ttyclock->framewin);
     wrefresh(ttyclock->framewin);

     /* Frame win move */
     mvwin(ttyclock->framewin, (ttyclock->geo.x = x), (ttyclock->geo.y = y));
     wresize(ttyclock->framewin, (ttyclock->geo.h = h), (ttyclock->geo.w = w));

     box(ttyclock->framewin, 0, 0);
     wrefresh(ttyclock->framewin);

     return;
}

void
set_second(void)
{
     int new_w = SECFRAMEW;
     int y_adj;

     for(y_adj = 0; (ttyclock->geo.y - y_adj) > (COLS - new_w - 1); ++y_adj);

     clock_move(ttyclock->geo.x, (ttyclock->geo.y - y_adj), new_w, ttyclock->geo.h);

     set_center();

     return;
}

void
set_center(void)
{
          clock_move((LINES / 2 - (ttyclock->geo.h / 2)),
                     (COLS  / 2 - (ttyclock->geo.w / 2)),
                     ttyclock->geo.w,
                     ttyclock->geo.h);
          wborder(ttyclock->framewin, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
}

int
main(int argc, char **argv)
{
     int c;

     /* Alloc ttyclock */
     ttyclock = malloc(sizeof(ttyclock_t));

     /* Default color */
     ttyclock->option.color = COLOR_BLUE; 

     while ((c = getopt(argc, argv, "sh:C:")) != -1)
     {
          switch(c)
          {
          case 'h':
          default:
               printf("usage :      clock [-h] [-C [0-7]]                               \n"
                      "    -C [0-7]      Set the clock color                            \n"
                      "    -h            Show this page                                 \n");
               free(ttyclock);
               exit(EXIT_SUCCESS);
               break;
					case 'C':
							 if(atoi(optarg) >= 0 && atoi(optarg) < 8)
							      ttyclock->option.color = atoi(optarg);
          }
     } 
     init();

     while(ttyclock->running)
     {
          update_hour();
          draw_clock();
     }

     free(ttyclock);
     endwin();

     return 0;
}

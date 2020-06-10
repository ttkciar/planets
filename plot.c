#include "planets.h"

#define xnputs(s,n) tputs(s, n, xputc)
#define xputs(s)    tputs(s, 1, xputc)
#define cls()       xnputs(CL, LI)
#define curs(x, y)  xputs(tgoto(CM, x, y))

extern char **environ;

#ifdef _GTTY_COMPAT_
struct  sgttyb_compat _tty;
#else
struct sgttyb _tty;
#endif

int xputc(char);

static char tbuf[512];
static char *CL, *CM;
static int CO, LI;

char
initscreen()
{
	char *tptr;
	char *tbufptr, *pc;

	tptr = (char *) malloc(1024);
	tbufptr = tbuf;

#ifdef _GTTY_COMPAT_
	ioctl(0, TIOCGETD, &_tty);
#else
	gtty(0, &_tty);
#endif

	if(tgetent(tptr, getenv("TERM")) < 1) {
		puts("Unknown terminal type");
		free(tptr);
		return (0);
	}

	pc = tgetstr("pc", &tbufptr);
    if (pc) {
    	PC = *pc;
    }

	CO = tgetnum ("co");
	LI = tgetnum ("li");
	CL = tgetstr ("cl", &tbufptr);
	CM = tgetstr ("cm", &tbufptr);
	free(tptr);
	return (CL && CM);
}

int
do_plot()
{
	static char flag;
	int max_x, max_y, min_x, min_y;
	int center;
	register int i;
	register struct planet *p;

	if (ac-1) {
		center = (int) strtol(av[1], NULL, 10);
		if (center < 0 || center >= NUM_PLANETS) {
			puts("Invalid planet number");
			return(1);
		}
	}
	else {
		center = ((emp == -1) ? 0 : game.empires[emp].e_first);
	}
	if (!flag) {
		flag = initscreen();  /* from csr.h, NOT curses.h */
	}
	if (!flag) {
		puts ("Terminal must have clearscreen and cursor movement");
		return(1);
	}
	lseek (fd, PLANET(0), 0);
	read (fd, game.planets, sizeof(game.planets));
	p = &game.planets[center];
	max_x = p->p_x + (CO / 4) - 2;
	min_x = p->p_x - (CO / 4) + 4;
	max_y = p->p_y + (LI / 2) - 3;
	min_y = p->p_y - (LI / 2) + 1;
	cls();
	if (min_x < 0 || max_x >= (MAX_X+1)) {

		int x = (((min_x < 0) ? -1 : (MAX_X + 1)) - min_x) * 2;

		for (i=((min_y < 0) ? -min_y : 0); (i < (LI-2) && i < (MAX_Y - min_y)); i++) {
			curs(x, i);
			putchar('|');
		}
	}
	if (min_y < 0 || max_y >= MAX_Y) {
		int endline = ((MAX_X - min_x + 1) * 2);
		curs((i = ((min_x < 0) ? ((-min_x) -1) : 0) * 2), (((min_y < 0) ? -1 : MAX_Y) - min_y));
		if (min_x < 0) putchar('+'), i++;
		for (; ((i < CO) && (i < endline)); i++) {
			putchar('-');
		}
		if (max_x >= (MAX_X+1)) {
			putchar('+');
		}
	}
	for (i=0; i < NUM_PLANETS; i++) {

		int x = game.planets[i].p_x;
		int y = game.planets[i].p_y;

		if (x < min_x || x > max_x || y < min_y || y > max_y) {
			continue;
		}
		curs((x - min_x) * 2, (y - min_y));
		printf("%02d", i);
		if (game.planets[i].p_emp == emp && emp != -1) {
			putchar ('*');
		}
	}
	curs (0, LI-2);
	return(0);
}

int
xputc(char c)  /* an actual function for tputs */
{
	putchar(c);
	return 0;
}

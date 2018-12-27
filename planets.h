#include <ctype.h>
#include <curses.h>
#include <fcntl.h>
#include <math.h>
#include <pwd.h>
#include <setjmp.h>
#include <sgtty.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <term.h>
#include <termcap.h>
#include <unistd.h>

extern char *playdir, *helpdir;
extern uid_t master_uid, real_uid;
#define MASTER (real_uid == master_uid)

#define DATA_FILE "data"

#define NUM_EMPIRES 30
#define NUM_PLANETS 100
#define NUM_SHIPS 600

struct empire {
	char e_name[20];
	int e_uid;
	int e_first;
};

struct planet {
	int p_x;
	int p_y;
	char p_name[23];
	char p_emp, p_claim;
	int p_util, p_prod, p_def, p_tech, p_res;
};

struct ship {
	char s_type;        /* b, s, c */
	char s_emp;         /* empire which ownes this */
	char s_seen;        /* has ship been detected? */
	char s_name[15];    /* obvious */
	float s_x;          /* current x, y position   */
	float s_y;
	float s_dest_x;     /* destination x, y position */
	float s_dest_y;
	float s_dist;       /* how far to destination    */
	int s_dest;         /* planet number */
	int s_mode;         /* 0(rest), 1(to planet), 2(to coords) */
	int s_points;       /* defense, stealth, reserve */
	int s_tech;         /* technology (speed)        */
};

/* values for s_mode */
#define ATREST		0
#define TOPLANET	1
#define TOCOORDS	2

struct {
	struct {
		long up_last;     /* last time a turn was done */
		long up_time;     /* time between turns in seconds */
		int up_num;       /* how many turns so far in game */
		int ship_top;     /* highest ship number used */
		int max_fleet;    /* max number of ships a player can have */
	} hdr;
	struct empire empires[NUM_EMPIRES];
	struct planet planets[NUM_PLANETS];
	struct ship ships[NUM_SHIPS];
} game;

struct {
	char *playdir;
	uid_t master_uid;
} game_config;

#define HISTORY 23
char *history[HISTORY];
int comnum;

#define MAX_ALIASES 50
struct {
	char *a_str;
	char *a_val;
} aliases[MAX_ALIASES];
int atop;

#define MAX_DEPTH 6
FILE *fstack[MAX_DEPTH];
int ftop;
char prompt[25];

char *av[20];
int ac;

char verbose, *home;
long time();
long seed;
int emp;
uid_t uid;
void ctrl_c(int);
int fd;

#define EMPIRE(x)	(long)((sizeof(game.hdr)) + ((x) * sizeof(struct empire)))
#define PLANET(x)	(long)((sizeof(game.hdr)) + sizeof(game.empires)\
						+ ((x) * sizeof(struct planet)))
#define SHIP(x)	    (long)((sizeof(game.hdr)) + sizeof(game.empires)\
						+ sizeof(game.planets) + ((x) * sizeof(struct ship)))

#define COST_SCOUT		100
#define COST_COLONY		200
#define COST_BATTLE		500
#define COST_DEF		50
#define COST_PROD		15
#define COST_STEALTH	50
#define COST_TECH(x)	(100*(x))

#define COST_S_DEF(amt,p)		((amt) * (50  - (p).p_tech/2))
#define COST_S_STEALTH(amt,p)	((amt) * (50  - (p).p_tech/2))
#define COST_S_TECH(amt,p)		((amt) * (100 - (p).p_tech/2))

#define PLANET_OUTPUT(p)  (((p)->p_prod-((p)->p_def/5.0))*(((p)->p_tech+0.5)/2.0))

#define SQR(x)			((x)*(x))
#define DIST(x1,y1,x2,y2)	(float)sqrt(SQR((x1)-(x2))+SQR((y1)-(y2)))
#define ABS(x) 			((x) > 0 ? (x) : -(x))

#define MAX_X 60
#define MAX_Y 120

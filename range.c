#include "planets.h"

struct rngstr {
	int rnum;
	float rdist;
};

int testum(const void *, const void *);

int
do_range() {
	struct rngstr rng[NUM_PLANETS];
	int amt, pnum, i;

	if (ac > 3) {
		puts("usage: range planet_num [how_many]");
		return(1);
	}

	if (ac == 1) {
		pnum = (emp == -1) ? 0 : game.empires[emp].e_first;
		amt = 10;
	}
	else {
		pnum = (int) strtol(av[1], NULL, 10);
		if (pnum < 0 || pnum >= NUM_PLANETS) {
			puts("Invalid planet number.");
			return(1);
		}
		amt = (ac == 2) ? 10 : (int) strtol(av[2], NULL, 10);
		if (amt < 0 || amt >= NUM_PLANETS) {
			puts("Invalid range.");
			return(1);
		}
	}

	for(i=0; i < NUM_PLANETS; ++i) {
		rng[i].rnum = i;
		rng[i].rdist = DIST((float)game.planets[i].p_x, 
			(float)game.planets[i].p_y, (float)game.planets[pnum].p_x, 
			(float)game.planets[pnum].p_y );
	}

	qsort( rng, NUM_PLANETS, sizeof(struct rngstr), testum );

	puts("No. Name            ( X,   Y)    Dist");
	for(i=0; i < amt; ++i) {
		printf("%2d  %-15s (%2d, %3d)   %5.2f",
			rng[i].rnum,
			game.planets[rng[i].rnum].p_name,
			game.planets[rng[i].rnum].p_x, 
			game.planets[rng[i].rnum].p_y, rng[i].rdist );
		if (((game.planets[rng[i].rnum].p_emp == emp) || MASTER)  && emp != -1)
			printf(" %6d", game.planets[rng[i].rnum].p_res);
		putchar('\n');
	}
	return(0);
}

int
testum(const void *p1, const void *p2)
{
	struct rngstr *r1 = (struct rngstr *) p1;
	struct rngstr *r2 = (struct rngstr *) p2;
	if (r1->rdist > r2->rdist) return(1);
	if (r1->rdist == r2->rdist) return(0);
	return(-1);
}

#include <memory.h>
#include <mhash.h>
#include "planets.h"

int lock(int);
void unlock();

int
do_build() {
	int pnum, amt;
	struct planet *p;
	char maxmode = !strcmp("max",av[0]);
	char allmode = 0;

	if (!maxmode && ac != 4) {
		puts("Usage: build <p,d,t> planet_num amount");
		return(1);
	}

	if (maxmode && ac != 3) {
		puts("Usage: max <p,d,t> planet_num");
		return(1);
	}
	if (!strcmp(av[2], "*")) allmode = 1;
	pnum = (allmode) ? 0 : atoi(av[2]);
	amt = atoi(av[3]);
	if (amt < 0) {
		puts("Cannot increase by a negative amount");
		return(1);
	}

	if (pnum < 0 || pnum >= NUM_PLANETS) {
		printf("No such planet\n");
		return(1);
	}

	if (!lock(1)) return(1);
	lseek(fd, PLANET(0), 0);
	read(fd, game.planets, sizeof(game.planets));

	for (;pnum < NUM_PLANETS; pnum++) {

		p = &game.planets[pnum];

		if (p->p_emp != emp && !MASTER) {
			if (allmode) continue;
			printf("You do not own planet #%d\n", pnum);
			unlock();
			return(1);
		}

		if (!strcmp(av[1], "p")) {
			if (maxmode) {
				amt = p->p_res / COST_PROD;
				if (amt > (p->p_util - p->p_prod)) amt = p->p_util - p->p_prod;
			}

			if (p->p_res >= COST_PROD * amt && amt + p->p_prod <= p->p_util) {
				p->p_prod += amt;
				p->p_res -= COST_PROD * amt;
			} else {
				if (p->p_prod + amt > p->p_util) {
					puts("Cannot increase beyond utility.");
				}
				else {
					puts("Too Expensive");
					unlock();
					return(1);
				}
			}
		}
		else if (!strcmp(av[1], "d")) {
			if (maxmode) {
				amt = p->p_res / COST_DEF;
				if (amt > (p->p_util - p->p_def)) amt = p->p_util - p->p_def;
			}

			if (p->p_res >= COST_DEF * amt && p->p_def + amt <= p->p_util) {
				p->p_def += amt;
				p->p_res -= COST_DEF * amt;
			} else {
				if (p->p_def + amt > p->p_util)
					puts("Cannot increase beyond utility.");
				else {
					puts("Too Expensive");
					unlock();
					return(1);
				}
			}
		}
		else if (!strcmp(av[1], "t")) {
			if (maxmode) amt = 10000;
			while( amt ) {
				if (p->p_res >= COST_TECH(p->p_tech+1) &&
												p->p_tech+1 <= p->p_util) {
					p->p_res -= COST_TECH(++p->p_tech);
					if (p->p_tech-8 > 0)
						p->p_util += p->p_tech - 8;
				} else {
					if (maxmode) break;
					if (p->p_tech + amt > p->p_util)
						puts("Cannot increase beyond utility");
					else
						puts("Too Expensive");
					unlock();
					return(1);
				}
				--amt;
			}
		}
		if (!allmode) break;
	}
	lseek(fd, PLANET(0), 0);
	write(fd, game.planets, sizeof(game.planets) );
	unlock();
	return(0);
}

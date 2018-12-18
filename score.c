#include "planets.h"

struct score {
	int t_emp;
	int t_planets, t_util, t_ships;
	long t_worth, t_invest;
	long t_tot;
};

int scorecmp();

do_score(fp)
FILE *fp;
{
	int i, j;
	int aflg = (ac == 2);
	struct score tot[NUM_EMPIRES];

	fputs("Who              Planets (Util)   Ships   Worth   Investment  Total\n", fp);
	lseek(fd, 0L, 0);
	read(fd, &game, sizeof(game));
	for(i=0; i < NUM_EMPIRES; ++i) {
		tot[i].t_planets = tot[i].t_util = tot[i].t_ships = 0;
		tot[i].t_worth = tot[i].t_invest = 0L;
		for(j=0; j < NUM_PLANETS; ++j) {
			if (i == game.planets[j].p_emp) {
				tot[i].t_planets += 1;
				tot[i].t_util += game.planets[j].p_util;
				tot[i].t_worth += game.planets[j].p_res;
				tot[i].t_invest += (long)(game.planets[j].p_prod * COST_PROD)
					+ (long)(game.planets[j].p_def * COST_DEF)
					+ (long)(game.planets[j].p_tech *
						(long)(game.planets[j].p_tech + 1) * 50);
			}
		}
		for(j=0; j < NUM_SHIPS; ++j) {
			if (i == game.ships[j].s_emp) {
				tot[i].t_ships += 1;
				switch (game.ships[j].s_type) {
					case 'c' :
						tot[i].t_invest += ((long)COST_COLONY + 
								(long)(game.ships[j].s_points));
						break;
					case 'b' :
						tot[i].t_invest += (COST_BATTLE + 
								(long)(game.ships[j].s_points * COST_DEF));
						break;
					case 's' :
						tot[i].t_invest += (COST_SCOUT + 
								(long)(game.ships[j].s_points * COST_STEALTH));
						break;
				}
			}
		}
		tot[i].t_emp = i;
		tot[i].t_tot = (long)tot[i].t_worth + (long)tot[i].t_invest;
	}

	qsort( tot, NUM_EMPIRES, sizeof(struct score), scorecmp);

	for(i=0; i < NUM_EMPIRES; ++i) {
		if (game.empires[tot[i].t_emp].e_uid != -1
			&& (aflg || tot[i].t_planets || tot[i].t_ships) )
			fprintf(fp, "%-20s %3d (%4d) %5d %9ld %9ld %9ld\n",
				game.empires[tot[i].t_emp].e_name,
				tot[i].t_planets,
				tot[i].t_util,
				tot[i].t_ships,
				tot[i].t_worth,
				tot[i].t_invest,
				tot[i].t_tot);
		}
		return(0);
}

scorecmp(s1, s2)
struct score *s1, *s2;
{
	if (s1->t_tot < s2->t_tot) return 1;
	if (s1->t_tot > s2->t_tot) return -1;
	return 0;
}

#include "planets.h"

void checklog();
void conflict(int, int);
int lock(int);
int logmsg(int, char *, ...);
void turn();
int read_tele(int, char);
int rnd(int);
void unlock();

int def_ships[NUM_SHIPS];
int num_def_ships;

int
check_turn() {
	int l;
	long now = time(NULL);
	static int beenhere = 0;
	static long lasttime;
	int tabort = 0;

	lseek(fd, 0L, 0);
	read(fd, &game.hdr, sizeof(game.hdr) );

	if (!beenhere) {
		lasttime = game.hdr.up_last;
		++beenhere;
	}

	if (game.hdr.up_time == 0) {
	    puts("Game is not initialized, aborting.");
	    return 1;
	}
	if ((l = (int)((now - game.hdr.up_last) / game.hdr.up_time)) > 0) {
		if ( lock(0) ) {
			fflush(stdout);
			if ( !fork() ) {
				lseek(fd, 0L, 0);
				read(fd, &game, sizeof(game));
				printf("Doing %d turn%c\n", l, l > 1 ? 's' : ' ');
				fflush(stdout);
				while( now - game.hdr.up_last >= game.hdr.up_time ) {
					turn();
					game.hdr.up_last += game.hdr.up_time;
					++game.hdr.up_num;
				}
				lseek(fd, 0L, 0);
				write(fd, &game, sizeof(game));
				unlock();
				_exit(0);
			}
		}
		else
			tabort = 1;
		signal(SIGINT, ctrl_c);
	}

	if (!l && game.hdr.up_last != lasttime) {
		puts("There has been a turn.");
		lseek(fd, 0L, 0);
		read(fd, &game, sizeof(game));
		if (emp != -1) {
			checklog();
			read_tele(emp, 0);
		}
	}
	lasttime = game.hdr.up_last;

	if (verbose) {
		long sec = game.hdr.up_last + game.hdr.up_time - time();
		long min = sec / 60, hour = 0;
		sec %= 60;
		
		if (min > 60) {
			hour = (min / 60);
			min %= 60;
		}
		printf("Next turn in ");
		if (hour) printf("%ld hr ", hour);
		if (min) printf("%ld min ", min);
		printf("%ld sec\n", sec);
	}

	return tabort;
}

void
turn()
{
	int i;
	register struct ship *s;
	register struct planet *p;
	register int j;

	for(i = -1; i < NUM_EMPIRES; ++i)
		logmsg((char) i, "\n** Turn #%d **\n", game.hdr.up_num);

	for(i=0; i < NUM_PLANETS; ++i) {
		int ship_dist(), shipno[NUM_SHIPS];
		size_t attackers = 0;

		num_def_ships = 0;
		p = &game.planets[i];

		p->p_res += PLANET_OUTPUT(p);

		for (j=0; j < game.hdr.ship_top; j++) {

			s = &game.ships[j];
			if (s->s_emp == -1 ) continue;
			if ((s->s_emp == p->p_emp || s->s_emp == p->p_claim)
				&& s->s_mode == 0 && s->s_dest == i)
						def_ships[num_def_ships++] = j;
			if (s->s_mode != 0) {
				float sx, sy;

				if (s->s_dist > 0.0) {
					if (s->s_dist > s->s_tech) {
						sx = s->s_x + (s->s_dest_x - s->s_x)
										* (s->s_tech / s->s_dist);
						sy = s->s_y + (s->s_dest_y - s->s_y)
										* (s->s_tech / s->s_dist);
					} else {
						sx = s->s_dest_x;
						sy = s->s_dest_y;
					}
				}
				else {
					sx = s->s_x;
					sy = s->s_y;
				}
				if (p->p_emp != -1 && (s->s_emp != p->p_emp)
					&& (s->s_seen == -1 || s->s_seen == i)) {

					int tech = p->p_tech;
					int stealth = s->s_points *
						((s->s_type == 's') ? 1 : 0);
					char seen = (s->s_seen == i);
					char *shiptype;

					switch (s->s_type) {
						case 'b': shiptype = "battle"; break;
						case 'c': shiptype = "colony"; break;
						case 's': shiptype = "scout"; break;
						default : shiptype = "unidentified";
					}

					if ((DIST(sx, sy, (float)p->p_x, (float)p->p_y) <= tech) && ((rnd(100 + stealth) <= (tech + (10 * seen))))) {
						
						if (!seen) {
							logmsg(p->p_emp,
							"\nA %s ship has been detected at (%2.0f, %3.0f)\n",
								shiptype, sx, sy, i);
							s->s_seen = (char) i;
						} else if ((int)sx != (int)s->s_x
												|| (int)sy != (int)s->s_y)
							logmsg(p->p_emp,
							"\nThe %s ship at (%2.0f, %3.0f) moved to (%2.0f, %3.0f)\n",
							shiptype, s->s_x, s->s_y, sx, sy);
					} else {
						if (seen) logmsg(p->p_emp,
							"\nThe %s ship at (%2.0f, %3.0f) has been lost\n",
							shiptype, s->s_x, s->s_y);
						s->s_seen = -1;
					}
				}
				if (s->s_dest == i && s->s_dist <= (float)s->s_tech)
					shipno[attackers++] = j;

				else if (i == NUM_PLANETS - 1) {
					if (s->s_dist <= s->s_tech) {
						if (s->s_mode == 2) {
							s->s_x = s->s_dest_x;
							s->s_y = s->s_dest_y;
							s->s_mode = 0;
							s->s_dist = 0.0;
							logmsg(s->s_emp,
								"\nShip #%02d has arrived at (%2.0f, %2.0f)\n",
								j, s->s_x, s->s_y);
						}
					} else {
						s->s_x = sx;
						s->s_y = sy;
						s->s_dist =
							DIST(s->s_x, s->s_y, s->s_dest_x, s->s_dest_y);
					}
				}
			}
		}
		if (!attackers) continue;  /* if there are no attacking ships */

		qsort(shipno, attackers, sizeof(int), ship_dist);

		for (j=0; j < attackers; j++) {
			s = &game.ships[shipno[j]];
			s->s_x = s->s_dest_x;
			s->s_y = s->s_dest_y;
			s->s_seen = -1;
			s->s_mode = 0;
			s->s_dist = 0.0;
			if (s->s_emp == p->p_emp) {
				logmsg(s->s_emp,
					"\nShip #%02d arrived at your planet #%d.\n", shipno[j], i);
				def_ships[num_def_ships++] = j;
			}
			else conflict(shipno[j], i);
		}
	}
}

/* called by qsort(), used to determine which ships
 * will arrive at the planet first.
 */
int
ship_dist(int *a, int *b) {
	struct ship *s1 = &game.ships[*a], *s2 = &game.ships[*b];
	int time1 = (int) ((s1->s_dist / s1->s_tech) * 10.0);  /* time to get to */
	int time2 = (int) ((s2->s_dist / s2->s_tech) * 10.0); /*     planet. */

	if (time1 < time2) return(-1);
	if (time1 > time2) return(1);
	if (s1->s_type == 'b') return(-1);  /* battle ships arrive first */
	if (s2->s_type == 'b') return(1);   /* if all else is equal. */
	return(0);  /* They arrive in ship-number order */
}

int
do_turn()
{
	if (!lock(1)) return 1;
	lseek(fd, 0L, 0);
	read(fd, &game, sizeof(game));
	turn();
	game.hdr.up_last = time();
	game.hdr.up_num++;
	lseek(fd, 0L, 0);
	write(fd, &game, sizeof(game));
	unlock();
	return(0);
}

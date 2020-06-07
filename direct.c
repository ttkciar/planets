#include "planets.h"

int lock(int);
void unlock();

int
do_direct() {
	int shpno, pnum;
	struct ship *s;

	if (ac != 3 && ac != 4) {
		puts("usage: direct ship_num < planet_num | x y >");
		return(1);
	}
	shpno = (int) strtol(av[1], NULL, 10);
	if (shpno < 0 || shpno >= NUM_SHIPS) {
		printf("%d not a valid ship number\n", shpno);
		return(1);
	}
	if (game.ships[shpno].s_emp != emp && !MASTER) {
		puts("You do not own that ship");
		return(1);
	}

	if (!lock(1)) return(1);
	lseek(fd, SHIP(shpno), 0);
	read(fd, &game.ships[shpno], sizeof(struct ship) );

	s = &game.ships[shpno];

	if (ac == 3) {
		pnum = (int) strtol(av[2], NULL, 10);
		if (pnum < 0 || pnum >= NUM_PLANETS) {
			printf("%d not a valid planet number\n", pnum);
			unlock();
			return(1);
		}
		s->s_dest = pnum;
		s->s_mode = 1;
		s->s_dest_x = (float) game.planets[pnum].p_x;
		s->s_dest_y = (float) game.planets[pnum].p_y;
		s->s_dist = DIST( s->s_x, s->s_y, s->s_dest_x, s->s_dest_y );
	} else {
		int x = (int) strtol(av[2], NULL, 10), y = (int) strtol(av[3], NULL, 10);
		if (x < 0 || x >= MAX_X || y < 0 || y >= MAX_Y) {
			puts("Cannot direct a ship out of known space.");
			unlock();
			return(1);
		}
		s->s_dest = -1;
		s->s_mode = 2;
		s->s_dest_x = (float) x;
		s->s_dest_y = (float) y;
		s->s_dist = DIST(s->s_x, s->s_y, s->s_dest_x, s->s_dest_y);
	}
	lseek(fd, SHIP(shpno), 0);
	write(fd, &game.ships[shpno], sizeof(struct ship) );
	printf("Your ship has been directed to (%2.0f, %3.0f)\n", s->s_dest_x,
			s->s_dest_y);
	unlock();
	return(0);
}

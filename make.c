#include "planets.h"

do_make() {
	int pnum;
	int amt, i;
	char type, name[15];

	if (ac < 2 || av[2][1]) {
		puts("usage: make planet_num <s,b,c> [units]");
		return(1);
	}

	pnum = atoi(av[1]);
	amt = av[3] ? atoi(av[3]) : 0;
	if (amt < 0) amt = 0;

	type = av[2][0];
	if (game.planets[pnum].p_emp != emp && !MASTER) {
		puts("You do not own that planet.");
		return;
	}

    if (!lock(1)) return;

	lseek(fd, 0L, 0);
	read(fd, &game, sizeof(game));

	for(i=0; i < NUM_SHIPS; ++i) 
		if (game.ships[i].s_emp == -1)
			break;

	if (i == NUM_SHIPS) {
		puts("There are too many ships in the game.");
		unlock();
		return(1);
	}

	if (i == game.hdr.ship_top) game.hdr.ship_top++;

	if (type == 'b') {
		int cost = ((COST_BATTLE - (game.planets[pnum].p_tech * 3)) + 
					amt * (COST_DEF - (game.planets[pnum].p_tech / 2)));

		if (game.planets[pnum].p_res < cost) {
			puts("Insufficient reserves to create battle ship");
			unlock();
			return(1);
		}
		if (amt > 35) {
			puts("Battle ships have a maximum defense of 35.");
			unlock();
			return(1);
		}
		game.planets[pnum].p_res -= cost;
		sprintf(name, "Battle_%02d", i);
	}
	else if (type == 'c') {
		int cost = ((COST_COLONY - (game.planets[pnum].p_tech * 2)) + amt);

		if (game.planets[pnum].p_res < cost) {
			puts("Insufficient reserves to create colony ship");
			unlock();
			return(1);
		}
		game.planets[pnum].p_res -= cost;
		sprintf(name, "Colony_%02d", i);
	}
	else if (type == 's') {
		int cost = ((COST_SCOUT - game.planets[pnum].p_tech) + 
					amt * (COST_STEALTH - (game.planets[pnum].p_tech / 4)));

		if (game.planets[pnum].p_res < cost) {
			puts("Insufficient reserves to create scout ship");
			unlock();
			return(1);
		}
		if (amt > 35) {
			puts("Scout ships have a maximum stealth of 35");
			unlock();
			return(1);
		}
		game.planets[pnum].p_res -= cost;
		sprintf(name, "Scout_%02d", i);
	}
	else {
		puts("Usage: make <planet num> <b|c|s> <points>");
		unlock();
		return(1);
	}
	game.ships[i].s_type = type;
	game.ships[i].s_x = game.ships[i].s_dest_x = game.planets[pnum].p_x;
	game.ships[i].s_y = game.ships[i].s_dest_y = game.planets[pnum].p_y;
	game.ships[i].s_emp = emp;
	game.ships[i].s_points  = amt;
	game.ships[i].s_tech = game.planets[pnum].p_tech;
	game.ships[i].s_mode = 0;
	game.ships[i].s_dest = pnum;
	game.ships[i].s_dist = 0.0;
	game.ships[i].s_seen = -1;
	strcpy(game.ships[i].s_name, name);
	lseek(fd, 0L, 0);
	write(fd, &game, sizeof(game));
	unlock();
	if (verbose) ship_hdr(stdout), pr_ship(stdout, i);
	return(0);
}

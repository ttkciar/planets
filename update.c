#include "planets.h"

int lock(int);
void unlock();

int
do_update() {
	int shipno, amt;
	struct ship *s;

	if (ac!=4 || (strcmp("r",av[1])!=0 && strcmp("d",av[1])!=0
					&& strcmp("t",av[1])!=0 && strcmp("s", av[1])!=0)) {
		puts("usage: update [r|d|t|s] ship_num amount");
		return(1);
	}

	shipno = (int) strtol(av[2], NULL, 10);
	if (shipno < 0 || shipno >= NUM_SHIPS) {
		puts("no such ship");
		return(1);
	}
	if (!lock(1)) return(1);
	s = &game.ships[shipno];
	lseek(fd, SHIP(shipno), 0);
	read(fd, s, sizeof( struct ship ) );

	if (s->s_emp != emp && !MASTER) {
		printf("You do not own ship #%d\n", shipno );
		unlock();
		return(1);
	}

	if (s->s_mode != 0 || s->s_dest == -1) {
		printf("Ship #%d is not at rest at a planet\n", shipno);
		unlock();
		return(1);
	}

	lseek(fd, PLANET(s->s_dest), 0);
	read(fd, &game.planets[s->s_dest], sizeof( struct planet ) );

	if (game.planets[s->s_dest].p_emp != s->s_emp) {
		printf("You do not own planet #%d\n", s->s_dest);
		unlock();
		return(1);
	}

	if (av[1][0] == 'r') {
		if (s->s_type != 'c') {
			printf("Ship #%d is not a Colony ship\n", shipno );
			unlock();
			return(1);
		}
		if (!strcmp(av[3], "+")) {
			s->s_points += game.planets[s->s_dest].p_res;
			game.planets[s->s_dest].p_res = 0;
		}
		else if (!strcmp(av[3], "-")) {
			game.planets[s->s_dest].p_res += s->s_points;
			s->s_points = 0;
		}
		else {
			amt = (int) strtol(av[3], NULL, 10);
			if (amt > 0 && amt > game.planets[s->s_dest].p_res) {
				puts("Planet does not have enough resources");
				unlock();
				return(1);
			}
			if (amt < 0 && -amt > s->s_points) {
				puts("Ship does not have enough resources");
				unlock();
				return(1);
			}
			game.planets[s->s_dest].p_res -= amt;
			s->s_points += amt;
		}
	}
	else if (av[1][0] == 'd') {
		int cost;

		if (s->s_type != 'b') {
			printf("Ship #%d is not a Battle ship\n", shipno );
			unlock();
			return(1);
		}
		if ((amt = (int) strtol(av[3], NULL, 10)) <= 0) {
			puts("Defense can only be increased.");
			unlock();
			return(1);
		}
		if (s->s_points + amt > 35) {
			puts("Battle ships have a maximum defense of 35");
			unlock();
			return(1);
		}
		cost = COST_S_DEF(amt, game.planets[s->s_dest]);
		if (cost > game.planets[s->s_dest].p_res) {
			puts("Not enough reserves on planet");
			unlock();
			return(1);
		}
		s->s_points += amt;
		game.planets[s->s_dest].p_res -= cost;
	}
	else if (av[1][0] == 's') {
		int cost;

		if (s->s_type != 's') {
			printf("Ship #%d is not a Scout ship\n", shipno );
			unlock();
			return(1);
		}
		if ((amt = (int) strtol(av[3], NULL, 10)) <= 0) {
			printf("Stealth can only be increased.");
			unlock();
			return(1);
		}
		if (s->s_points + amt > 35) {
			puts("Scout ships have a maximum stealth of 35");
			unlock();
			return(1);
		}
		cost = COST_S_STEALTH(amt, game.planets[s->s_dest]);
		if (cost > game.planets[s->s_dest].p_res) {
			puts("Not enough reserves on planet");
			unlock();
			return(1);
		}
		s->s_points += amt;
		game.planets[s->s_dest].p_res -= cost;
	}

	else if (av[1][0] == 't') {
		int cost;

		if ((amt = (int) strtol(av[3], NULL, 10)) <= 0) {
			printf("Technology can only be increased.");
			unlock();
			return(1);
		}
		cost = COST_S_TECH(amt, game.planets[s->s_dest]);
		if (cost > game.planets[s->s_dest].p_res) {
			puts("Not enough reserves on planet");
			unlock();
			return(1);
		}
		s->s_tech += amt;
		game.planets[s->s_dest].p_res -= cost;
	}

	lseek(fd, SHIP(shipno), 0);
	write(fd, s, sizeof( struct ship ) );
	lseek(fd, PLANET(s->s_dest), 0);
	write(fd, &game.planets[s->s_dest], sizeof( struct planet ) );
	unlock();
	return(0);
}

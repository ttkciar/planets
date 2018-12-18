#include "planets.h"
#include <pwd.h>

do_hdr() {
	lseek(fd, 0L, 0);
	read(fd, &game.hdr, sizeof(game.hdr) );
	printf("game.hdr.up_last = %ld\n", game.hdr.up_last );
	printf("game.hdr.up_time = %ld\n", game.hdr.up_time );
	printf("game.hdr.up_num  = %d\n", game.hdr.up_num );
	printf("game.hdr.max_fleet = %d\n", game.hdr.max_fleet );
	printf("game.hdr.ship_top  = %d\n", game.hdr.ship_top );
}

do_empires() {
	int i, u;
	lseek(fd, 0L, 0);
	read(fd, &game, sizeof(game));

	for(i=0; i < NUM_EMPIRES; ++i) {
		u = game.empires[i].e_uid;
		printf( "%2d %-20s %4d", i, game.empires[i].e_name, u);
		if (ac != 1 && u != -1)
			printf("  %s", getpwuid(game.empires[i].e_uid)->pw_name);
		putchar('\n');
	}
	return 0;
}

do_planets(out)
FILE *out;
{
	int i;
	lseek(fd, PLANET(0), 0);
	read(fd, game.planets, (NUM_PLANETS * sizeof(struct planet)) );
	if (ac == 2) {
		i = atoi(av[1]);
		if (i < 0 || i >= NUM_PLANETS) {
			puts("Bad planet number.");
			return 1;
		}
		if (game.planets[i].p_emp == emp || MASTER) {
			planet_hdr(out);
			pr_planet(out, i);
			return 0;
		}
		else {
			puts("You do not own that planet.");
			return 1;
		}
	}

	planet_hdr(out);
	for(i=0; i < NUM_PLANETS; ++i)
		if (game.planets[i].p_emp==emp || (MASTER&&game.planets[i].p_emp!= -1))
			pr_planet(out, i);
	return 0;
}

do_ship(out)
FILE *out;
{
	int i;
	if (ac != 2) {
		puts("usage: ship ship_no");
		return 1;
	}
	i = atoi(av[1]);
	if (i < 0 || i >= NUM_SHIPS) {
		puts("bad ship number");
		return 1;
	}
	lseek(fd, SHIP(i), 0);
	read(fd, &game.ships[i], sizeof(struct ship) );

	if (game.ships[i].s_emp == emp || MASTER) {
		ship_hdr(out);
		pr_ship(out, i);
		return 0;
	}
	else {
		puts("You do not own that ship.");
		return 1;
	}
}

do_fleet(out)
FILE *out;
{
	int i, p = -1;
	int flg;

	lseek(fd, SHIP(0), 0);
	read(fd, game.ships, (NUM_SHIPS * sizeof(struct ship)) );

	p = -1;
	if (ac == 1)
		flg = 0;
	else if (ac == 3 && !strcmp(av[1], "-d")) {
		flg = 'd';
		p = atoi(av[2]);
	}
	else if (ac == 2 && !strcmp(av[1], "-b"))
		flg = 'b';
	else if (ac == 2 && !strcmp(av[1], "-c"))
		flg = 'c';
	else if (ac == 2 && !strcmp(av[1], "-s"))
		flg = 's';
	else if (ac == 2) {
		flg = 0;
		p = atoi(av[1]);
	}
	else {
		puts("usage:");
		puts("\tfleet");
		puts("\tfleet planet_num");
		puts("\tfleet -d planet_num");
		puts("\tfleet -[bcs]");
		return 1;
	}

	if (p != -1 && (p < 0 || p >= NUM_PLANETS) ) {
		puts("bad planet number");
		return 1;
	}

	if (MASTER) fputs("Emp ", out);
	ship_hdr(out);
	for(i=0; i < NUM_SHIPS; ++i)
		if ((game.ships[i].s_emp != -1)
			&& (MASTER || (game.ships[i].s_emp == emp))) {

			if ( ((!flg||flg=='d') && game.ships[i].s_mode == 0 && game.ships[i].s_dest == p)
				|| (!flg && p == -1)
				|| (flg=='d' && game.ships[i].s_mode==1 && game.ships[i].s_dest==p)
				|| (flg == game.ships[i].s_type)
				) {

				if (MASTER) fprintf(out, "%2d  ", game.ships[i].s_emp);
				pr_ship(out, i);

			}
		}
	return 0;
}

do_name()
{
	if (ac == 2 || (ac == 4 && strcmp(av[1], "-s")) || ac > 4) {
		puts("Usage: name planet_num new_planet_name");
		puts("Or:    name -s ship_num new_ship_name");
		return 1;
	}
	
	if (ac == 1)
		puts(game.empires[emp].e_name);
	else if (ac == 3) {
		int i = atoi(av[1]);

		if (i < 0 || i >= NUM_PLANETS) {
			puts("Invalid planet number.");
			return 1;
		}

		if (!lock(1)) return;
		lseek(fd, PLANET(i), 0);
		read(fd, &game.planets[i], sizeof(struct planet));
		if (game.planets[i].p_emp != emp) {
			puts("You can only name planets you own.");
			unlock();
			return 1;
		}
		strncpy(game.planets[i].p_name, av[2], 15);
		game.planets[i].p_name[15] = 0;
		lseek(fd, PLANET(i), 0);
		write(fd, &game.planets[i], sizeof(struct planet));
		unlock();
	}
	else {
		int i = atoi(av[2]);

		if (i < 0 || i >= NUM_SHIPS) {
			puts("Invalid ship number.");
			return 1;
		}

		if (!lock(1)) return;
		lseek(fd, SHIP(i), 0);
		read (fd, &game.ships[i], sizeof(struct ship));
		if (game.ships[i].s_emp != emp) {
			puts("You can only name ships you own.");
			unlock();
			return 1;
		}
		strncpy(game.ships[i].s_name, av[3], 15);
		game.ships[i].s_name[14] = 0;
		lseek(fd, SHIP(i), 0);
		write(fd, &game.ships[i], sizeof(struct ship));
		unlock();
	}
	return 0;
}

do_dist() {
	int p1, p2;
	if (ac != 3) {
		puts("usage: dist planet_num planet_num");
		return(1);
	}
	p1 = atoi(av[1]);
	p2 = atoi(av[2]);
	if (p1 < 0 || p1 > NUM_PLANETS) {
		printf("bad planet number %d\n", p1);
		return 1;
	}
	if (p2 < 0 || p2 > NUM_PLANETS) {
		printf("bad planet number %d\n", p2);
		return 1;
	}
	printf("dist = %4.2f\n",
		DIST((float)game.planets[p1].p_x, (float)game.planets[p1].p_y,
		(float)game.planets[p2].p_x, (float)game.planets[p2].p_y));
	return 0;
}

do_scan() {
	int i, p = -1;

	lseek(fd, SHIP(0), 0);
	read(fd, game.ships, sizeof(game.ships));

	if (ac != 1) {
		puts("usage: scan");
		return 1;
	}

	scan_hdr();
	for(i=0; i < NUM_SHIPS; ++i) {
			int j = (int)game.ships[i].s_seen;

			if (j > -1 && game.planets[j].p_emp == emp)
				pr_scan(i);
	}
	return 0;
}

do_un_nuke() {
	int n = atoi(av[1]);
	if (ac == 1) {
		puts("usage: planet_num name emp x y util prod def tech res");
		return 1;
	}
	if (!lock(1)) return;
	lseek(fd, PLANET(n), 0);
	read(fd, &game.planets[n], sizeof(struct planet) );
	strcpy(game.planets[n].p_name, av[2]);
	game.planets[n].p_emp = atoi(av[3]);
	game.planets[n].p_x = atoi(av[4]);
	game.planets[n].p_y = atoi(av[5]);
	game.planets[n].p_util = atoi(av[6]);
	game.planets[n].p_prod = atoi(av[7]);
	game.planets[n].p_def = atoi(av[8]);
	game.planets[n].p_tech = atoi(av[9]);
	game.planets[n].p_res = atoi(av[10]);
	lseek(fd, PLANET(n), 0);
	write(fd, &game.planets[n], sizeof( struct planet) );
	unlock();
	return 0;
}

do_move() {
	int num;
	if (ac != 4) {
		puts("usage: move planet_num newx newy");
		return 1;
	}
	num = atoi(av[1]);
	if (num < 0 || num >= NUM_PLANETS) {
		puts("bad planet number");
		return 1;
	}
	if (!lock(1)) return 1;
	lseek(fd, PLANET(num), 0);
	read(fd, &game.planets[num], sizeof(struct planet) );
	game.planets[num].p_x = atoi(av[2]);
	game.planets[num].p_y = atoi(av[3]);
	lseek(fd, PLANET(num), 0);
	write(fd, &game.planets[num], sizeof(struct planet) );
	unlock();
	return 0;
}

do_lfiles() {
	int i;
	if (ac != 1) {
		puts("usage: lfiles");
		return 1;
	}
	for(i = -1; i < NUM_EMPIRES; ++i)
		init_log(i);
	return 0;
}

do_tfiles() {
	int i;
	if (ac != 1) {
		puts("usage: tfiles");
		return 1;
	}
	for(i = -1; i < NUM_EMPIRES; ++i)
		init_tele(i);
	return 0;
}

do_rename()
{
	int emp;
	if (ac != 3) {
		puts("Usage: rename <empire number> <new name>");
		return(1);
	}

	if (!lock(1)) return(1);

	emp = atoi(av[1]);
	lseek(fd, EMPIRE(emp), 0);
	read(fd, &game.empires[emp], sizeof(struct empire));
	strcpy(game.empires[emp].e_name, av[2]);
	lseek(fd, EMPIRE(emp), 0);
	write(fd, &game.empires[emp], sizeof(struct empire));
	unlock();
}

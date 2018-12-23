#include <zconf.h>
#include <mhash.h>
#include "planets.h"

int init_log(int);
void init_tele(int);
int lock(int);
int rnd(int);
void unlock();

char *names[] = {"Pyruss",     "Tattooine",  "Magrathea",  "Vogon",
				 "Skaros",     "Talos IV",   "Urras",      "Skywatch",
				 "Trantor",    "Vulcan",     "Planet 10",  "Krypton",
				 "Anacreon",   "Gondor",     "Durin",      "Romulus",
				 "Caprica",    "Fantasia",   "Pelnor",     "Dragonor",
				 "Appleton",   "Bayport",    "Krikkit",    "Hrung",
				 "Betelgeuse", "Goofball",   "Nerf",       "Narcissus",
				 "Potamia",    "Alderaan",   "Dagobah",    "Zinfandel",
				 "Xyzzy",      "Narnia",     "Melnibone",  "Immyrmyr",
				 "Dantooine",  "Sirius",     "Alphax",     "Anthrax",
				 "Yondor",     "Rhun",       "Marin",      "Orange",
				 "Planet X",   "Napa",       "Caladan",    "Pervect",
				 "Imper",      "Deevil",     "Smirnoff",   "Bantir",
				 "Splat",      "Disney",     "Frogstar",   "Hellhole",
				 "Neoprax",    "Ringworld",  "Gallifrey",  "Latvia",
				 "Arrakis",    "Styx",       "Olympus",    "R'lyeh",
				 "Oledean",    "Xanth",      "Riverworld", "Kobal",
				 "Moria",      "Dunwich",    "Bismuth",    "Jasmine",
				 "Xanadu",     "Shangri-La", "Deneb",      "Rigel",
				 "Oz",         "Greyhawk",   "Hollywood",  "Atlantis",
				 "Halley's",   "Brojitnsk",  "Phoenix",    "Midgard",
				 "Yaziria",    "Nirvana",    "GiediPrime", "Basketball",
				 "Genesis",    "Shalar",     "Jhereg",     "Flatland",
				 "Moscow",     "Roq",        "Maraschino", "Grimm",
				 "Lovecraft",  "Peace",      "Defense 92", "Kendel" };

int
do_init() {
	int i, c, NUM_NAMES = sizeof(names) / sizeof(char *);

	printf("Do you really want to re-initialize everything? ");
	fflush(stdout);

	if (! ((c = getchar()) == 'y' || c == 'Y')) {
		while( getchar() != '\n');
		printf("Initialization aborted\n");
		return 0;
	}
	while( getchar() != '\n');
	printf("Initializing Everything...\n");
	fflush(stdout);

	if (!lock(1)) return(1);

	/* make log and tele files for manager */
	init_log(-1);
	init_tele(-1);

	for(i=0; i < NUM_EMPIRES; ++i) {
		game.empires[i].e_name[0] = 0;
		game.empires[i].e_uid = -1;
		init_tele(i);
		init_log(i);
	}

	for (i=0; i<NUM_NAMES; i++) {
		int j = rnd(NUM_NAMES);
		char *temp;

		if (j == 10 || i == 10) continue;

		temp = names[i];
		names[i] = names[j];
		names[j] = temp;
	}

	for(i=0; i < NUM_PLANETS; ++i) {
		int j;
		char good = 0;
		if (NUM_NAMES < NUM_PLANETS) {
			puts("Not enough names for planets.");
			fflush(stdout);
			close(fd);
			unlock();
			exit(-8);
		}
		else strcpy(game.planets[i].p_name,  names[i]);
		game.planets[i].p_emp = -1;
		game.planets[i].p_claim = -1;
		while (!good) {
			register struct planet *p1, *p2;
			good = 1;
			p1 = &game.planets[i];
			p1->p_x = rnd(MAX_X);
			p1->p_y = rnd(MAX_Y);
			for(j=0; j < i; j++) {
				p2 = &game.planets[j];
				if ((p1->p_x == p2->p_x && p1->p_y == p2->p_y)
								|| (ABS(p1->p_y - p2->p_y) <= 3
								&& ABS(p1->p_x - p2->p_x) <= 3)) {
					good = 0;
					break;
				}
			}
		}

		if (rnd(10) > 5) {
			/* uninhabited */
			game.planets[i].p_util = rnd(40) + 60;
			game.planets[i].p_prod = 0;
			game.planets[i].p_def = 0;
			game.planets[i].p_tech = 1;
			game.planets[i].p_res = rnd(200) + 50;
		}
		else {
			/* inhabited */
			game.planets[i].p_util = rnd(40) + 60;
			game.planets[i].p_prod = rnd(20) + 10;
			game.planets[i].p_def = rnd(5) + 1;
			game.planets[i].p_tech = rnd(3) + 1;
			game.planets[i].p_res = rnd(2000) + 1000;
		}
	}

	for(i=0; i < NUM_SHIPS; ++i) {
		game.ships[i].s_emp = -1;
		game.ships[i].s_seen = -1;
		game.ships[i].s_name[0] = 0;
	}

	game.hdr.up_last = time();
	game.hdr.up_time = 21600L;   /* six - hour default */
	game.hdr.up_num = 0;
	game.hdr.max_fleet = NUM_SHIPS;
	game.hdr.ship_top = 0;
	lseek(fd, 0L, 0);
	write( fd, &game, sizeof(game) );
	unlock();
	return(0);
}

int
do_enroll() {
	int num, i;
	int unused[NUM_PLANETS];
	int utop=0;
	char buf[BUFSIZ], *c;

	if (ac != 1 && !MASTER) {
		puts("Usage: enroll");
		return(1);
	}
	if (!lock(1)) return(1);
	lseek(fd, 0L, 0);
	read(fd, &game, sizeof(game) );

	if (MASTER) {
		int uid_enroll;

		if (ac < 3 || (((emp = atoi(av[2])) != -1) && ac == 3) || ac > 4) {
			puts("Usage: enroll <uid> <empire> <planet>");
			unlock();
			return(1);
		}
		emp = atoi(av[2]);
		for(i=0; i < NUM_PLANETS; ++i) {
			if (game.planets[i].p_emp == emp) {
				game.planets[i].p_emp = -1;
			}
		}
		uid_enroll = atoi(av[1]);
		for(i=0; i < NUM_EMPIRES; ++i) {
			if (game.empires[i].e_uid == uid_enroll) {
				break;
			}
		}
		if (i != NUM_EMPIRES) {
			game.empires[i].e_uid = -1;
		}
		if (emp != -1) {
			game.empires[emp].e_uid = uid_enroll;
			i = atoi(av[3]);
			game.empires[emp].e_first = i;
			game.planets[i].p_emp = (char) emp;
		}
		else {
			lseek(fd, 0L, 0);
			write(fd, &game, sizeof(game));
			unlock();
			return(0);
		}
	} else {
		for(i=0; i < NUM_EMPIRES; ++i) {
			if (game.empires[i].e_uid == -1) {
				unused[utop++] = i;
			}
			if (game.empires[i].e_uid == uid) {
				break;
			}
		}

		if (!utop) {
			puts("Planets is full -- too many players");
			unlock();
			return(1);
		}

		if (game.empires[i].e_uid == uid) {
			puts("You are already enrolled.");
			unlock();
			return(1);
		}

		emp = unused[rnd(utop)];

		game.empires[emp].e_uid = uid;

		for(num=0, i=0; i < NUM_EMPIRES; ++i) {
			if (game.empires[i].e_uid != -1) ++num;
		}

		game.hdr.max_fleet = NUM_SHIPS / ((!num) ? 1 : num);

		utop=0;
		for(i=0; i < NUM_PLANETS; ++i) {
			if (game.planets[i].p_emp == -1) {
				unused[utop++] = i;
			}
		}

		if (!utop) {
			puts("Planets is full -- no planets left");
			unlock();
			return(1);
		}

		i = unused[rnd(utop)];
		game.empires[emp].e_first = i;
		game.planets[i].p_emp = (char) emp;

	}
	fputs("Enter a name for your empire (up to 20 characters): ", stdout);
	fflush(stdout);

	while ((c = gets(buf)) != NULL) {
		size_t len = strlen(buf);
		int j;
		char good = 1;

		for (j=0; j<len; j++) {
			if (buf[j] < 32) {
				good = 0;
			}
		}
		if (good) {
			break;
		}
	}
	if (c == NULL) {
		buf[0] = 0;
	}
	strcpy(game.empires[emp].e_name, buf);
	if (verbose) {
		printf("The %s empire has been created on planet #%d.\n",
			game.empires[emp].e_name, i);
	}
	game.planets[i].p_res  = rnd(1000) + 4000;
	if (game.planets[i].p_def == 0) {
		game.planets[i].p_def  += rnd(5) + 1;
		game.planets[i].p_tech += rnd(2) + 1;
		game.planets[i].p_prod += rnd(20) + 10;
	}
	lseek(fd, 0L, 0);
	write(fd, &game, sizeof(game) );
	unlock();
	if (MASTER) emp = -1;
	return(0);
}

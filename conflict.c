#include "planets.h"

extern int num_def_ships;  /* these defined in turn.c */
extern int def_ships[];

conflict (shipno, planetno)
{
	register struct ship *s = &game.ships[shipno];
	register struct planet *p = &game.planets[planetno];
	int i;

	logmsg(s->s_emp, "\n%s Ship #%02d arrived at %s planet #%02d\n",
			s->s_type=='b' ? "Battle" : s->s_type=='s' ? "Scout" : "Colony",
			shipno, 
			(p->p_emp == -1 && p->p_def == 0) ? "unoccupied" :
				(p->p_emp == -1) ? "primitive" : "inhabited",
			planetno);

	switch (s->s_type) {
		case 's' : 
			{
			int count = 0, shpn = s->s_emp;

			for (i=0; i < num_def_ships; i++) {
				struct ship *s2;
				if (def_ships[i] == -1) continue;
				s2 = &game.ships[def_ships[i]];

				if ( s2->s_type == 'b' ) {
					if (rnd(50 + s2->s_tech) > s->s_points) {
						logmsg(s->s_emp, 
							"Your scout was destroyed %s on planet #%02d\n",
							"by defending fleet", planetno);
						logmsg(s2->s_emp, 
							"Your battle ship destroyed a scout on planet #%02d\n",
							planetno);
						s->s_emp = -1;
						break;
					}
					++count;
				}
				else if ( s2->s_type == 's' ) {
					if (rnd(50 + s2->s_tech) > s->s_points) {
						logmsg(s2->s_emp, 
							"\nYour scout ship %s on planet #%02d\n",
							"detected an enemy scout", planetno);
					}
					if (rnd(50 + s->s_tech) > s2->s_points) {
						logmsg(shpn, 
							"Your scout ship %s on planet #%02d\n",
							"detected an enemy scout", planetno);
					}
				}
			}

			if (count) {
				logmsg (shpn, "There are %d battle ships on planet #%02d\n",
					count + rnd(2), planetno);
				if (s->s_emp == -1) return;
			}

			if ( (p->p_def > 0)
				&& ((rnd(50) + 2 * p->p_tech + 10 * (s->s_seen == planetno))
				> s->s_points) ) {
					logmsg(shpn,
						"Your scout was destroyed on planet #%02d\n",
						planetno);
					logmsg(p->p_emp,
						"\nA scout was destroyed on planet #%02d\n", planetno);
					s->s_emp = -1;
			}
			else s->s_seen = -1;

			logmsg (shpn,
				"Planet #%d has utility %d and defense %d\n", 
				planetno, p->p_util + rnd(10) - 5,
				p->p_def ? p->p_def + rnd(3) : 0);

			if (p->p_emp != -1)
				logmsg (shpn, "Planet #%d is owned by %s\n", planetno,
					game.empires[p->p_emp].e_name);

			break;
			}
		case 'c' :
			if (p->p_def > 0) { /* planet is defended */
				logmsg(s->s_emp, 
					"Ship #%d was captured on planet %02d\n",
					shipno, planetno);
				if (p->p_emp != -1)
					logmsg(p->p_emp, 
						"\nA colony ship was captured on your planet #%d\n", 
					 	planetno);
				s->s_emp = p->p_emp;
			} else if ((p->p_emp != -1 || p->p_claim != -1)
					&& p->p_claim != s->s_emp && num_def_ships > 0) {

				/* planet is claimed and there's a ship to back it up */

				for(i=0; i<num_def_ships; i++) {
					struct ship *s2 = &game.ships[def_ships[i]];

					if (s2->s_type == 'b' && s2->s_points > 0) {
						logmsg(s->s_emp, 
							"Ship #%d destroyed by a battleship on planet #%02d\n",
							shipno, planetno);

						if (p->p_claim != -1) logmsg(p->p_claim, 
							"\nYour battleship destroyed a colony ship on planet #%02d\n", 
					 			planetno);
						else if (p->p_emp != -1) logmsg(p->p_emp, 
							"\nYour battleship destroyed a colony ship on planet #%02d\n", 
					 			planetno);
						s->s_emp = -1;
						break;
					}
				}
			} else {
				if (num_def_ships) { /* there must be a scout or two here */
					int i;

/* This assumes that if there are ships on an unclaimed and unowned planet,
   they must be scouts.  Seems like a good assumpton for the time being... */

					for (i=0; i < num_def_ships; i++) {

						register struct ship *s2;
						
						if (def_ships[i] == -1) continue;

						s2 = &game.ships[def_ships[i]];

						if (s2->s_emp == s->s_emp || s2->s_emp == -1) continue;
							/* Looks for an ENEMY scout */

						logmsg(s2->s_emp, 
							"A colony ship has landed on planet #%02d\n",
							planetno);
						if (rnd(50) + s->s_tech > s2->s_points)
						logmsg(s->s_emp, 
							"A scoutship was found on planet #%02d\n",
							planetno);
					}
				}
				logmsg(s->s_emp, "Ship #%d colonized planet #%02d\n",
					shipno, planetno);
				p->p_emp = s->s_emp;
				p->p_claim = -1;
				s->s_emp = -1;
				p->p_res += s->s_points;
			}
			break;
		case 'b' : {
			int def, i;
			float mult_ship = (float)(1 + ((float)(rnd(20) - 10) / 100));
			float mult_plan = (float)(1 + ((float)(rnd(20) - 5) / 100));

			/* if (p->p_def == 0 && p->p_claim == -1) {
				p->p_claim = s->s_emp;
				if (p->p_emp != -1) {
					logmsg (p->p_emp, "Planet #%02d DESTROYED!\n", planetno);
					p->p_emp = -1;
				}
				return;
			} */

			if (p->p_claim == s->s_emp) {
				def_ships[num_def_ships++] = shipno;
				break;
			}

			if (p->p_emp != -1) logmsg(p->p_emp,
				"\nIncoming battleship on planet #%02d!\n", planetno);

			for (i=0; i < num_def_ships; i++) {

				register struct ship *s2;
				
				if (def_ships[i] == -1) continue;

				s2 = &game.ships[def_ships[i]];

				if (s2->s_type == 's' && s2->s_emp != p->p_emp
						&& s2->s_emp != s->s_emp) {

					logmsg(s2->s_emp, 
						"\nA battleship landed on planet #%02d\n", planetno);
					if (rnd(50 + s->s_tech) > s2->s_points) {
						logmsg(s->s_emp, 
							"\nA scout was destroyed on planet #%02d\n",
												planetno);
						logmsg(s2->s_emp, 
							"Your scout was destroyed on planet #%02d\n",
												planetno);
						s2->s_emp = -1; /* scout is caught and destroyed */

					} else s2->s_dest = -1; /* scout leaves */

					continue;
				}
				else if (s2->s_type != 'b') continue;
							/* a colony, we'll deal with it later */
				def = s->s_points;
				s->s_points -= (s2->s_points * mult_ship);
				s2->s_points -= (def * mult_plan);
				if (s2->s_points <= 0) {
					s2->s_points = 0;
					logmsg(s2->s_emp, "Defending battleship #%d destroyed!\n",
								def_ships[i]);
					s2->s_emp = -1;
					def_ships[i] = -1;
				} else {
					if (p->p_emp != -1) 
						logmsg (p->p_emp, "Your fleet destroyed a %s #%02d\n",
							"battleship on planet", planetno);
					logmsg(s->s_emp,
						"Ship #%02d destroyed by defending fleet\n", shipno);
					s->s_points = 0;
					s->s_emp = -1;
					return;
				}
			}

			def = s->s_points;
			s->s_points -= (p->p_def * mult_ship);
			p->p_def -= (def * mult_plan);

			if (p->p_def > 0) {
				logmsg (p->p_emp, 
					"A battleship was destroyed on planet #%02d\n", planetno);
				logmsg (s->s_emp, 
					"Ship #%d was destroyed on planet #%02d\n", 
						shipno, planetno);
				s->s_emp = -1;
			} else {
				if (s->s_points <= 0) s->s_points = 0;
				logmsg (s->s_emp, "Ship #%d destroyed planet #%02d\n",
					shipno, planetno);
				if (p->p_emp != -1)
					logmsg (p->p_emp, "Planet #%02d DESTROYED!\n", planetno);
				p->p_emp = -1;
				p->p_tech -= (def / 10) * mult_plan;
				p->p_util -= (def / 3)  * mult_plan;
				p->p_prod -= (def / 4)  * mult_plan;
				p->p_res  -= (def * 7)  * mult_plan;
				p->p_tech = (p->p_tech < 0) ? 0 : p->p_tech;
				p->p_util = (p->p_util < 0) ? 0 : p->p_util;
				p->p_prod = (p->p_prod < 0) ? 0 : p->p_prod;
				p->p_res  = (p->p_res  < 0) ? 0 : p->p_res;
				if (p->p_prod > p->p_util) p->p_prod = p->p_util;
				p->p_def = 0;
				p->p_claim = s->s_emp;

				for (i=0; i < num_def_ships; i++) {
					struct ship *s2;
					if (def_ships[i] != -1) {
						s2 = &game.ships[def_ships[i]];
						s2->s_emp = s->s_emp;
					}
				}

			}
		}
	}
}

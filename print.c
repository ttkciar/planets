#include "planets.h"

planet_hdr(out)
FILE *out;
{
	fputs(" # Name       Empire               ( X,   Y)  Util  Prod   Def  Tech  Reserve\n", out);
}

ship_hdr(out)
FILE *out;
{
	fputs("Type Name         Shp ( X,   Y) Status             Speed  Distance   Points\n", out);
}

pr_planet(out, i)
FILE *out;
{
	fprintf(out, "%2d %-10.10s ", i, game.planets[i].p_name);
	fprintf(out, "%-20.20s ", game.planets[i].p_emp == -1 ?
		"" : game.empires[game.planets[i].p_emp].e_name );
	fprintf(out,"(%2d, %3d)", game.planets[i].p_x, game.planets[i].p_y);
	fprintf(out,"%6d%6d%6d%6d%9d\n",
		game.planets[i].p_util,
		game.planets[i].p_prod,
		game.planets[i].p_def,
		game.planets[i].p_tech,
		game.planets[i].p_res );
}

pr_ship(out, i)
FILE *out;
{
	struct ship *s = &game.ships[i];
	char rbuf[30];

	if (MASTER && s->s_emp == -1) return;
	if (s->s_mode) {
		if (s->s_dest >= 0)
			sprintf(rbuf, "Route %2d @ (%2.0f, %3.0f)",
				s->s_dest, s->s_dest_x, s->s_dest_y);
		else
			sprintf(rbuf, "Route      (%2.0f, %3.0f)",
									s->s_dest_x, s->s_dest_y);
	}
	else {
		if (s->s_dest >= 0)
			sprintf(rbuf, "Rest  %2d", s->s_dest);
		else
			sprintf(rbuf, "Rest");
	}

	fprintf(out, "  %c  %-12.12s %3d (%2.0f, %3.0f) %-20.20s %3d    %6.2f    %5d\n",
		s->s_type - 32,
		s->s_name,
		i,
		s->s_x,
		s->s_y,
		rbuf,
		s->s_tech,
		s->s_dist,
		s->s_points);
}

pr_scan(i){
	struct ship *s = &game.ships[i];
	char rbuf[30];
	float sx, sy, range;

	lseek(fd, SHIP(0), 0);
	read(fd, game.ships, sizeof game.ships);

	if (!MASTER && s->s_emp == -1) return;

	if (s->s_dist > 0.0) {
		sx = s->s_x + (s->s_dest_x - s->s_x) * (s->s_tech / s->s_dist);
		sy = s->s_y + (s->s_dest_y - s->s_y) * (s->s_tech / s->s_dist);
	}
	else {
		sx = s->s_x;
		sy = s->s_y;
	}

	if (s->s_mode) {

		if (s->s_dest >= 0) {  /* is it going to a planet? */

			sprintf(rbuf, "Route %2d @ (%2.0f, %3.0f)",
							s->s_dest, s->s_dest_x, s->s_dest_y);
			range = s->s_dist;

		} else { /* no, so we can only tell where it will be next turn */

			sprintf(rbuf, "Route      (%2.0f, %3.0f)", sx, sy);
			range = s->s_tech;
		}
	}
	else {
		if (s->s_dest >= 0)
			return(0);
		else
			sprintf(rbuf, "Rest");
	}

	if (s->s_mode)
		printf("%-8.8s (%2.0f, %3.0f)  %-20.20s   %5d    %6.2f\n",
			(s->s_type == 's') ? "Scout" : ((s->s_type == 'b') ? 
				"Battle" : "Colony"),
			s->s_x,
			s->s_y,
			rbuf,
			s->s_tech,
			range);
	else
		printf("%-8.8s (%2.0f, %3.0f)  %-20.20s\n",
			(s->s_type == 's') ? "Scout" : ((s->s_type == 'b') ?
				"Battle" : "Colony"),
			s->s_x,
			s->s_y,
			rbuf);
}

scan_hdr()
{
	puts("Type     ( X,   Y)  Status                 Speed     Range");
}

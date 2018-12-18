#include "planets.h"

struct cmd{
	char *str;
	char priv;
	int (*func)();
};

int do_init(), do_help(), do_turn(), do_build(), do_planets(),
	do_fleet(), do_enroll(), do_make(), do_direct(), do_echo(),
	do_source(), do_alias(), do_exit(), do_plot(), do_range(), do_log(),
	do_interval(), unlock(), do_score(), do_update(), do_name(), do_set(),
	do_telegram(), do_universe(), do_empires(), do_next(), do_ship();
	do_dist(), do_scan(), do_un_nuke(), do_move(), do_lfiles(), do_tfiles(),
	do_history(), do_hdr(), do_rename(), do_max_fleet(), do_mark();

struct cmd cmds[] = {
	{"alias", 	0,	do_alias},
	{"planets",	1,	do_planets},
	{"direct", 	1,	do_direct},
	{"plot",	0,	do_plot},
	{"build", 	1,	do_build},
	{"update",	1,	do_update},
	{"fleet", 	1,	do_fleet},
	{"ship", 	1,	do_ship},
	{"range",	0,	do_range},
	{"make",	1,	do_make},
	{"max", 	1,	do_build},
	{"score",	0,	do_score},
	{"set", 	0,	do_set},
	{"next", 	0,	do_next},
	{"dist", 	0,	do_dist},
	{"help",	0,	do_help},
	{"quit", 	0,	do_exit},
	{"q", 		0,	do_exit},
	{"history",	0,	do_history},
	{"telegram",1,	do_telegram},
	{"scan", 	1,	do_scan},
	{"log", 	1,	do_log},
	{"name", 	1,	do_name},
	{"source", 	0,	do_source},
	{"enroll", 	0,	do_enroll},
	{"universe", 	1,	do_universe},
	{"echo",	0,	do_echo},
	{"init", 	2,	do_init},
	{"empires",	2,	do_empires},
	{"interval", 2,	do_interval},
	{"turn", 	2,	do_turn},
	{"un_nuke",	2,	do_un_nuke},
	{"move",	2,	do_move},
	{"tfiles",	2,	do_tfiles},
	{"lfiles",	2,	do_lfiles},
	{"hdr", 	2,	do_hdr},
	{"rename",	2,	do_rename},
	{"mark",	2,	do_mark},
	{"max_fleet",	2,	do_max_fleet},
	{"unlock", 	2,	unlock}
};

#define NUMCMDS (sizeof(cmds) / sizeof(struct cmd))

jmp_buf jmpenv;

commands()
{
	register char *s, *e;
	char combuf[256], abuf[256], *a1, *a2, *t;
	int i, n;

	signal(SIGINT, ctrl_c);

	fstack[0] = stdin;
	ftop = 1;
	atop = 0;
	verbose = 1;
	comnum = 0;
	check_turn();

	if (MASTER) strcpy(prompt, "=> ");
	else strcpy(prompt, "> ");

	sprintf(combuf, "%s/.planetsrc", home);
	if ((fstack[ftop] = fopen(combuf, "r")) != NULL)
		++ftop;

	if (setjmp(jmpenv))
		putchar('\n');

	for(;;) {
		if (fstack[ftop-1] == stdin) fputs(prompt, stdout);
		fflush(stdout);

		fgets(combuf, 80, fstack[ftop-1] );
		if (feof(fstack[ftop-1])) {
			fclose(fstack[--ftop]);
			if (!ftop) {
				close(fd);
				exit(1);
			}
			continue;
		}

		if (*combuf == '\n')
			continue;

		if (fstack[ftop-1] == stdin) {
			/*
			 * Copy a2 to a1 making history substitutions
			 * Both are terminated with \n\0
			 */
			a2 = combuf;
			a1 = abuf;
			if ((i = hist_sub(a2, a1)) == -1) continue;
			if (i == 1) {
				fputs(a1, stdout);
				fflush(stdout);
			}

			if (comnum >= HISTORY)
				free(history[comnum%HISTORY]);

			strcpy(history[comnum++%HISTORY]=malloc(strlen(a1)+1),a1);
		}
		else {
			a1 = combuf;
			a2 = abuf;
		}


		/*
		 * Copy a1 to a2 making alias substitutions
		 * Both are terminated with \n
		 */
		i = 0;
		while( (n = alias_sub(a1, a2)) ) {
			if (++i == 10) break;
			t = a1, a1 = a2, a2 = t;
		}

		if (n == -1)
			continue;

		if (i == 10) {
			puts("Alias too extensive.");
			continue;
		}

		/*
		 *  If this is a series of commands separated with semi-colons,
		 *  pass each individual command to execute().
		 *  If there is some sort of execution error, execute() will
		 *  return 1, and the rest of the compound command is aborted.
		 */
		s = e = a2;
		for(;;) {
			char tc;
			while( *e != '\n' && *e != ';') {
				if ( *e++ == '"' ) {
					while( *e++ != '"');
				}
			}
			tc = *e;
			*e = '\n';
			if (execute(s) || tc == '\n') {
				break;
			}
			s = ++e;
		}
	}
}

/*
 * execute()
 * This returns the value of the command's function unless there
 * was a problem generating *av[].  In this case, 1 is returned.
 * This allows the execution of a command with semi-colons to stop
 * when one of the individual commands returns an error status.
 * If the typed in command has semi-colons, each individual command
 * needs to be passed here.
 *
 */
execute(com)
char *com;
{
	register int i;
	FILE *out;

	if (make_argv(com) == -1)
		return 1;

	if (!strcmp(av[0], "logout")) {
		puts("Where do you think you are?");
		return 1;
	}

	if (ac == 0)
		return 0;

	out = stdout;
	if (av[ac-2][0] == '>' && !av[ac-2][1]) {
		char file[80];
		if (av[ac-1][0] != '/')
			sprintf (file, "%s/%s", home, av[ac-1]);
		else strcpy(file, av[ac-1]);
		if ((out = fopen(file, "w")) == NULL) {
			printf("cannot open '%s' for writing\n", file);
			return 1;
		}
		ac -= 2;
	}

	for(i=0; i < NUMCMDS; ++i)
		if (!strcmp(cmds[i].str, av[0])) {
			if (cmds[i].priv==0 || (cmds[i].priv==1 && emp>-1) || MASTER) {
				if (fstack[ftop-1] == stdin && cmds[i].func != do_exit
					&& cmds[i].func != do_init && cmds[i].func != do_mark)
					check_turn();
				i = (*cmds[i].func)(out);
				if (out != stdout) fclose(out);
				return i;
			}
			else if (emp == -1) {
				printf("%s: You are not enrolled.\n", av[0]);
				return 1;
			}
			else {
				printf("%s: Priviledged command.\n", av[0]);
				return 1;
			}
			break;
		}
	
	if (i == NUMCMDS) {
		printf("no such command '%s'\n", av[0]);
		return 1;
	}
}

ctrl_c() {
	signal(SIGINT, ctrl_c);
	stdout->_cnt = BUFSIZ;
	stdout->_ptr = stdout->_base;
	longjmp(jmpenv, 1);
}

/*
 * String is terminated with \n
 * Assumes quotes have already been checked.
 */
make_argv(s)
register char *s;
{
	static char avbuf[256];
	register char *d = avbuf;

	ac = 0;
	while( *s != '\n') {
		while(*s == ' ') ++s;
		if (*s == '\n') break;
		av[ac++] = d;
		if (*s == '"') {
			while( (*d++ = *++s) != '"' );
			*(d-1) = '\0';
			++s;
		}
		else {
			while(*s && *s != ' ' && *s != '\n')
				*d++ = *s++;
			*d++ = '\0';
		}
	}
	av[ac] = NULL;
	return 0;
}

do_help() {
	int fd, i, shown=0;
	char buf[BUFSIZ];

	char **argv = av;

	if (ac == 1) {
		for(i=0; i < NUMCMDS; ++i)
			if (MASTER || (cmds[i].priv != 2 &&
				(cmds[i].priv != 1 || emp != -1))){
				printf ("%-10s\t", cmds[i].str);
				if (!(++shown % 4)) putchar ('\n');
			}
		putchar ('\n');
	}
	else
		while( *++argv ) {
			sprintf(buf, "%s/%s", helpdir, *argv );

			fd = open(buf, 0);
			if (fd < 0)
				printf("Unable to find help on '%s'\n", *argv);
			else {
				while( (i = read(fd, buf, BUFSIZ)) > 0 )
					write(1, buf, i);
				close(fd);
			}
		}
}

do_mark() {
	if (!lock(1)) return 1;
	lseek(fd, 0L, 0);
	read(fd, &game.hdr, sizeof(game.hdr));
	game.hdr.up_last = time(0);
	lseek(fd, 0L, 0);
	write(fd, &game.hdr, sizeof(game.hdr));
	unlock();
}

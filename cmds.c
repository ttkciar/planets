#include "planets.h"

int lock(int);
void unlock();

int
do_alias() {
	int i;
	char abuf[80];
	register char *a = abuf, *s, **v = av+2, *m;

	if (ac == 1) {
		for(i=0; i < atop; ++i)
			printf("%s\t%s\n", aliases[i].a_str, aliases[i].a_val);
	}
	else if (ac == 2) {
		for(i=0; i < atop; ++i)
			if (!strcmp(av[1], aliases[i].a_str))
				break;
		if (i == atop) {
			puts("No such alias.");
			return 1;
		} else
			printf("%s\t%s\n", aliases[i].a_str, aliases[i].a_val);
	}
	else {
		for(i=0; i < atop; ++i)
			if (!strcmp(av[1], aliases[i].a_str))
				break;
		if (i != atop)
			free(aliases[i].a_val);
		else {
			m = malloc(strlen(av[1])+1);
			if (m == NULL) {
				puts("Out of Memory for Aliases");
				return 1;
			}
			strcpy(aliases[atop++].a_str = m , av[1]);
		}
		while( (s = *v++) ) {
			while( *s )
				*a++ = *s++;
			if (*v) *a++ = ' ';
		}
		*a = '\0';
		m = malloc(a - abuf + 1);
		if (m == NULL) {
			puts("Out of Memory for Aliases");
			free(aliases[--atop].a_str);
			return 1;
		}
		strcpy( aliases[i].a_val = m, abuf);
	}
	return 0;
}

int
do_source() {
	char file [80];

	if (ac != 2) {
		puts("usage: source file_name");
		return 1;
	}
	if (ftop == MAX_DEPTH - 1) {
		printf("sourcing can only go %d deep\n", ftop);
		return 1;
	}

	if (av[1][0] != '/') sprintf(file, "%s/%s", home, av[1]);
	else strcpy (file, av[1]);

	if ((fstack[ftop] = fopen(file, "r")) != NULL) {
		++ftop;
		return 0;
	}
	else {
		printf("source: cannot open [%s]\n", file);
		return 1;
	}
}

int
do_interval() {
	if (!lock(1)) return 1;
	lseek(fd, 0L, 0);
	read(fd, &game.hdr, sizeof(game.hdr));
	if (ac == 1)
		printf("time interval is %ld minutes\n", game.hdr.up_time / 60L);
	else if (ac == 2) {
		printf("new interval is %ld seconds\n",
			game.hdr.up_time = strtol(av[1], NULL, 10) * 60L );
	}
	lseek(fd, 0L, 0);
	write(fd, &game.hdr, sizeof(game.hdr));
	unlock();
	return 0;
}

int
do_exit() {
	if (ftop == 1) {
		close(fd);
		exit(1);
	}
	else
		fclose(fstack[ftop--]);
	return 0;
}

int
do_next() {
	long sec, min, hour;

	if (fstack[ftop-1] == stdin && verbose)/* if we did it before the command */
		return 0;

	lseek(fd, 0L, 0);
	read(fd, &game.hdr, sizeof( game.hdr ) );

	sec = game.hdr.up_last + game.hdr.up_time - time(NULL);
	min = sec / 60;
	hour = 0;
	sec %= 60;
		
	if (min > 60) {
		hour = (min / 60);
		min %= 60;
	}
	printf("Next turn in ");
	if (hour) printf("%ld hr ", hour);
	if (min) printf("%ld min ", min);
	printf("%ld sec\n", sec);
	
	return 0;
}

/*
 * This copies the string o to dest, and if the command will produce
 * an av[0] which is an alias, insert the alias value.
 * Each word after a ; is a av[0].
 * Both strings are terminated by \n
 * Assumes quotes are matched.
 */
int
alias_sub(register char *src, register char *dest)
{
	register char *a;
	char abuf[40], *src_start;
	int i, ret=0;

	while( *src != '\n' ) {
		a = abuf;
		while( *src == ' ' ) ++src;
		src_start = src;

		if (*src == '"') {		/* gross first char for av[0] */
			*a++ = *src++;
			while( (*a++ = *src++) != '"' );
		}
		else {
			while( *src != ' ' && *src != ';' && *src != '\n')
				*a++ = *src++;
		}
		*a = '\0';

		for(i=0; i < atop; ++i)
			if (!strcmp(abuf, aliases[i].a_str))
				break;

		if (i == atop) {
			while( src_start != src )
				*dest++ = *src_start++;
		}
		else {
			char *v = aliases[i].a_val;
			while( *v )
				*dest++ = *v++;
			ret = 1;
		}
		while( *src != ';' && *src != '\n') {
			if (*src == '"') {
				*dest++ = *src++;
				while( *src != '"' )
					*dest++ = *src++;
			}
			*dest++ = *src++;
		}
		if (*src == ';') *dest++ = *src++;
	}
	*dest = '\n';
	return ret;
}

int
do_set()
{
	if (ac == 1) {
		fputs("prompt\t", stdout);
		puts(prompt);
		if (verbose) puts("verbose");
		else puts("noverbose");
		return 0;
	}
	if (!strcmp(av[1], "prompt")) {
		if (ac < 3) {
			puts("Set prompt to what??");
			return 1;
		}
		else strncpy (prompt, av[2], 24);
		prompt[24] = 0;
		return 0;
	}
	if (!strcmp(av[1], "noverbose")) {
		verbose = 0;
		return 0;
	}
	if (!strcmp(av[1], "verbose")) {
		verbose = 1;
		return 0;
	}
	else {
		puts("Unknown variable.");
		return 1;
	}
}

/*
 * Both strings are terminated with \n\0
 * This checks for unmatched quotes
 */
int
hist_sub(char *s, char *d)
{
	char *t;
	int delta=0;

	if (*s == '^') {
		char *m, *l = history[(comnum-1) % HISTORY];
		++s;		/* s points to first char in pattern string */
		t = s;
		while ( *l ) {
			if (*l == *t) {
				m = l;		/* m is start of string being tested */
				while( *t != '^' && *m == *t)
					++m, ++t;
				if (*t == '^') {	/* found one */
					++t;
					while( *t != '^' && *t != '\n')
						*d++ = *t++;
					while( *m )
						*d++ = *m++;
					*d = '\0';
					return 1;
				}
				t = s;
			}
			*d++ = *l++;
		}
		puts("Modifier failed.");
		return -1;
	}

	while( *s ) {
		if (*s == '"') {
			*d++ = *s++;
			while( *s != '"' && *s)
				*d++ = *s++;
			if (*s != '"') {
				puts("Unmatched quote.");
				return -1;
			}
			*d++ = *s++;
		}
		else if (*s == '!') {
			++s;
			if (*s == '!') {
				++s;
				t = history[(comnum-1) % HISTORY];
				while( *t != '\n')
					*d++ = *t++;
				++delta;
			}
			else if (*s == '*') {
				++s;
				t = history[(comnum-1) % HISTORY];
				while( *t == ' ') ++t;
				while( *t != ' ' && *t != '\n') ++t;
				while( *t != '\n')
					*d++ = *t++;
				++delta;
			}
			else if (*s == '$') {
				char *l;

				++s;
				t = l = history[(comnum-1) % HISTORY];
				while( *t != '\n')
					++t;
				--t;
				while( *t == ' ' && t != l) --t;
				while( *t != ' ' && t != l) --t;
				while( *t != '\n')
					*d++ = *t++;
				++delta;
			}
			else if (isdigit(*s)) {
				int num;
				char nbuf[5], *n = nbuf;

				while (isdigit(*s)) *n++ = *s++;
				*n = 0;
				num = (int) strtol(nbuf, NULL, 10);
				if (num > (comnum - 1) || num < (comnum - HISTORY)) {
					printf("%d: event not found\n", num);
					return(-1);
				}
				t = history[num % HISTORY];
				while (*t != '\n') *d++ = *t++;
				++delta;
			}
			else {
				puts("Unknown history escape.");
				return -1;
			}
		}
		else
			*d++ = *s++;
	}
	*d = '\0';

	/*
	 * Terminal B at Cowell sometimes changes what you type
	 * to ^@, so check for that so we don't get a core dump later
	 * when the string is expected to end with \n.
	 */
	if (*(d-1) != '\n') {
		puts("line too bizarre");
		return -1;
	}

	return delta;
}

int
do_history() {
	int i;
	i = (comnum - HISTORY < 0) ? 0 : comnum - HISTORY;
	for(; i < comnum; ++i) {
		printf("%d\t%s", i, history[i%HISTORY] );
	}
	return 0;
}

int
do_max_fleet() {
	if (ac != 2) { puts("usage: max_fleet num"); return 1; }
	if (!lock(1)) return 1;
	lseek(fd, 0L, 0);
	read(fd, &game.hdr, sizeof(game.hdr));
	game.hdr.max_fleet = (int) strtol(av[1], NULL, 10);
	lseek(fd, 0L, 0);
	write(fd, &game.hdr, sizeof(game.hdr));
	unlock();
	return 0;
}

/*  And for completeness... */
int
do_echo() {
	char **a = av+1;
	bool newLine = true;

	if (!strcmp(av[1], "-n")) {
		newline = false;
		++a;
	}
	while (*a) printf("%s ", *a++);

	if (newline) putchar('\n');
	return 0;
}

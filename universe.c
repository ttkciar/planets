#include "planets.h"

#ifdef vax
#define PRINT_COM	"lpr -P%s -"
#else
#define PRINT_COM	"/bin/print -r=%s -"
#endif

int ycmp();

do_universe(out)
FILE *out;
{

	char outline[MAX_X*2 + 12];
	register	int i;
	register	int j;
	int x;
	int y;
	int len;
	char		num[10];
	char		cmd[30];
	int			sorty[NUM_PLANETS], s_top;
	int pfd[2], outfd, pid, status;

	if (ac > 2) {
		puts("Usage: 'universe <printer>' or 'universe > filename'");
		return(1);
	}

	if (out == stdout) {
		char c;
		if (ac == 1) {
			puts("Usage: 'universe <printer>' or 'universe > filename'");
			return(1);
		}
		printf("Do you really want to pay $0.09 cents? ");
		fflush (stdout);
		if ((c=getchar()) == 'n' || c == 'N')
			return 1;
		if (!strcmp(av[1], "narrow") || !strcmp(av[1],"ibm")) {
			puts("You cannot print there.");
			return 1;
		}
		while( getchar() != '\n');
		sprintf(cmd, PRINT_COM, av[1] );
	
		signal(SIGPIPE, SIG_IGN);

		pipe(pfd);

		if ( !(pid = fork()) ) {
			signal(SIGINT, SIG_IGN);
			/* we don't want the silly diagnostic print messages */
			close(1);
			close(2);
			open ("dev/null", 1);
			open ("dev/null", 1);
			chdir("/");
			dup2(pfd[0], 0);
	 		close(pfd[1]);
			execl("/bin/sh", "sh", "-c", cmd, 0);
			exit(1);
		}
		close (pfd[0]);
		outfd = pfd[1];
	}
	else
		outfd = fileno(out);

	/* Everything written to outfd will go to printer or file */

	/* Sort planets by Y coordinate */
	for(i=0; i < NUM_PLANETS; ++i)
		sorty[i] = i;

	qsort(sorty, NUM_PLANETS, sizeof(int), ycmp );

	outline[MAX_X*2+9] = '\n';
	for (x = 0; x < MAX_X*2+12; ++x) 
		outline[x] = ' ';

	/* Print vertical numbers 0-59 on the top two lines */
	write(outfd, "\n", 1);

	for (x = 0; x < MAX_X; ++x) 
		outline[5 + x*2] =  '0' + (x % 100) / 10;
	outline[6 + MAX_X*2] = '\n';

	write(outfd, outline, MAX_X*2+7);

	for (x = 0; x < MAX_X; ++x) 
		outline[5 + x*2] = '0' + x % 10;
	outline[6 + MAX_X*2] = '\n';

	write(outfd, outline, MAX_X*2+7);

	/* print a long one of these:  +------+ */
	outline[3] = '+';
	for(x=4; x < (MAX_X*2)+5; ++x)
		outline[x] = '-';
	outline[5 + MAX_X*2] = '+';
	outline[6 + MAX_X*2] = '\n';

	write(outfd, outline, MAX_X*2+7);

	for (x = 0; x < MAX_X*2+10; ++x) 
		outline[x] = ' ';

	s_top=0;
	for(y = 0; y < MAX_Y; ++y) {
		char nbuf[5];
		for(i=0; i < MAX_X*2+12; ++i)
			outline[i] = ' ';
		sprintf(nbuf, "%3d", y);
		strncpy(outline, nbuf, 3);
		strncpy(outline+MAX_X*2+6, nbuf, 3);
		outline[3] = '|';
		outline[MAX_X*2+5] = '|';
		while( s_top < NUM_PLANETS && game.planets[sorty[s_top]].p_y == y ) {
			sprintf(nbuf, "%02d%c", sorty[s_top],
				(game.planets[sorty[s_top]].p_emp == emp && emp != -1) ? 
											'*' : ' ');
			strncpy(outline + 4 + game.planets[sorty[s_top]].p_x * 2, nbuf, 3);
			++s_top;
		}
		outline[11 + MAX_X*2] = '\n';
		write(outfd, outline, MAX_X*2+12);
	}

	/* print a long one of these:  +------+ */
	for (x = 0; x < MAX_X*2+10; ++x) 
		outline[x] = ' ';

	outline[3] = '+';
	for(x=4; x < (MAX_X*2)+5; ++x)
		outline[x] = '-';
	outline[5 + MAX_X*2] = '+';
	outline[6 + MAX_X*2] = '\n';

	write(outfd, outline, MAX_X*2+7);

	/* Print vertical numbers 0-59 on the bottom two lines */
	for (x = 0; x < MAX_X*2+10; ++x) 
		outline[x] = ' ';

	for (x = 0; x < MAX_X; ++x) 
		outline[5 + x*2] =  '0' + (x % 100) / 10;
	outline[6 + MAX_X*2] = '\n';

	write(outfd, outline, MAX_X*2+7);

	for (x = 0; x < MAX_X; ++x) 
		outline[5 + x*2] = '0' + x % 10;
	outline[6 + MAX_X*2] = '\n';

	write(outfd, outline, MAX_X*2+7);

	close(outfd);

	if (out == stdout) {
		wait(&status);
		if (status>>8)
			printf("%s: unknown printer\n", av[1]);
		else
			puts("3 pages have been charged to your account.");
		signal(SIGPIPE, SIG_DFL);
	}

	return(0);
}

ycmp(n1, n2)
int *n1, *n2;
{
	if (game.planets[*n1].p_y > game.planets[*n2].p_y) return 1;
	if (game.planets[*n1].p_y < game.planets[*n2].p_y) return -1;
	return 0;
}

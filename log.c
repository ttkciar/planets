#include "planets.h"

/*
 * Print all or portions of the log file
 */
do_log() {
	FILE *fp;
	char buf[80], srch[40];
	int n;
	if (ac == 2) {
		n = atoi(av[1]);
		sprintf(srch, "planet #%02d", n);
	}
	else if (ac == 3 && !strcmp(av[1], "-s")) {
		n = atoi(av[2]);
		sprintf(srch, "Ship #%02d", n);
	}
	else if (ac != 1) {
		puts("usage: log [<-s ship_num> | <planet_numm>]");
		return(1);
	}

	sprintf(buf, "log_%02d", emp);

	fp = fopen(buf, "r");
	if (fp == NULL) {
		puts("cannot open datafile");
		return(1);
	}
	fseek(fp, (long )sizeof( long), 0);

	while( fgets(buf, 80, fp) ) {
		if (ac == 1)
			printf("%s", buf);
		else if (instr(srch, buf)) {
			putchar('\n');
			printf("%s", buf);
			if (!fgets(buf, 80, fp)) break;
			do {
				if (*buf == '\n') break;
				printf("%s", buf);
			} while( fgets(buf, 80, fp ) );
		}
	}

	fclose(fp);
	return(0);
}

/*
 * Instr() this returns true if srch is in str.
 */
instr( srch, str )
char *srch;
register char *str;
{
	register char *t = srch, *start;
	while( *str ) {
		if (*str == *t && *t) {
			start = str;
			while(*str == *t && *t) {
				++str;
				++t;
			}
			if (!*t) return 1;
			str = start;
			t = srch; 
		}
		++str;
	}
	return 0;
}

/*
 * Append a message to the log file of Empire 'emp'.
 */
logmsg(e_num, fmt, args)
char e_num, *fmt;
int args;
{
	FILE *fp;
	char file[40];

	sprintf(file, "log_%02d", e_num);
	if ((fp = fopen(file, "a")) == NULL) {
		printf("Couldn't open mail file for empire %d.  Mail games.", e_num);
		return(1);
	}
	_doprnt(fmt, &args, fp);
	fclose(fp);
	return(0);
}

/*
 * Print any new messages in the log file
 */
checklog()
{
	char file[40], buf[BUFSIZ];
	FILE *fp;
	int n;
	long where;

	sprintf(file, "log_%02d", emp);
	if ((fp = fopen(file, "r+")) == NULL) {
		puts("Couldn't open mail file.");
		return;
	}
	fread(&where, sizeof(long), 1, fp);
	fseek(fp, where, 0);

	n = 0;
	while((fgets(buf, 80, fp)) != NULL) {
		fputs(buf, stdout);
		if (++n % 15 == 0) {
			fputs("Press return to continue: ", stdout);
			fflush(stdout);
			gets(buf);
			if (buf[0] == 'q' || buf[0] == 'Q') break;
		}
	}

	where = ftell(fp);
	fseek(fp, 0L, 0);
	fwrite(&where, sizeof(long), 1, fp);
	fclose(fp);
}

/*
 * if flag == 0 print all new telegrams
 * if flag == 1 print all telegrams
 */
read_tele(num, flag)
int num;
char flag;
{
	char file[40], buf[BUFSIZ], *i, abort = 0;
	FILE *fp;
	long where;

	if (emp == -1 && !MASTER) return(1);
	sprintf(file, "tele_%02d", num);
	if ((fp = fopen(file, "r+")) == NULL) {
		puts("Couldn't open tele file.");
		return;
	}
	if (flag) where = sizeof(long);
	else {
		fread(&where, sizeof(long), 1, fp);
		fseek(fp, 0L, 2);
		if (where == ftell(fp)) {
			fclose(fp);
			return;
		}
	}

	fseek(fp, where, 0);

	if ((i = fgets(buf, BUFSIZ, fp)) == NULL) return;

	if (!flag) puts("--- Incoming Telegram ---");
	putchar ('\n');
	fflush(stdout);

	while(!abort) {
		Puts(buf);
		if ((i = fgets(buf, BUFSIZ, fp)) == NULL) abort = 1;
		if (!strncmp(buf, "From", 4) || abort) {
			char buffer[BUFSIZ];
			printf("Press return to continue: ");
			fflush(stdout);
			gets(buffer);
			if (buffer[0] == 'q' || buffer[0] == 'n') break;
			putchar ('\n');
		}
	}

	where = ftell(fp);
	fseek(fp, 0L, 0);
	fwrite(&where, sizeof(long), 1, fp);
	fclose(fp);
}

init_log(emp_num)
{
	char file[40];
	register int lfd;
	long where = sizeof(long);

	sprintf(file, "log_%02d", emp_num);
	unlink(file);
	if ((lfd = creat(file, 0666)) == -1) {
		puts("Couldn't create log file.");
		return;
	}
	write(lfd, &where, sizeof(long));
	close(lfd);
}

init_tele(emp_num) {
	char file[40];
	register int lfd;
	long where = sizeof(long);

	sprintf(file, "tele_%02d", emp_num);
	unlink(file);
	if ((lfd = creat(file, 0666)) == -1) {
		puts("Couldn't create telegram file.");
		return;
	}
	write(lfd, &where, sizeof(long));
	close(lfd);
}

Puts(s)
register char *s;
{
	while (*s) 
		if (*s < 32 && *s != '\n' && *s != '\t') 
				putchar('^'), putchar(*(s++) + 64);
		else putchar (*s++);
}
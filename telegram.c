#include "planets.h"

char buf[BUFSIZ];


int last_dot();
int read_in();
void headers(int num);
void inc_dot();
void saveall(FILE *tfp);
void tele_break();
void telecmds();
void type(int num);
void telegram();

int
do_telegram() {
	char telefile[40], *tmpfile = mktemp("/tmp/plXXXXXX");
	char subj[81];
	int tmp, i, j;
	ssize_t n;
	FILE *ftmp, *ftele;

	if (ac == 1) {
		if (MASTER || emp > -1) telegram();
		return(1);
	}
	if (ac < 2) {
			puts("usage: telegram [-a] [ Empire_Name ]");
			return(1);
	}
	if (!strcmp("-a", av[1]) && ac != 3) {
			puts("usage: telegram [-a] [ Empire_Name ]");
			return(1);
	}
	if (ac > 3) {
			puts("usage: telegram [-a] [ Empire_Name ]");
			return(1);
	}

	/* Now we try to send Mail to the Empire with name av[1] */

	if (!strcmp(av[1], "-a")) av[1] = av[2];

	for(i=0; i < NUM_EMPIRES; ++i)
		if (!strcmp(av[1], game.empires[i].e_name))
			break;

	if (!strcmp(av[1], "Players")) i = -2;

	if (!strcmp(av[1], "Manager") || !strcmp(av[1], "Mangler")) i = -1;

	if (i == NUM_EMPIRES) {
		puts("There is no such Empire");
		return(1);
	}

	tmp = creat(tmpfile, 0600);
	if (tmp < 0) {
		puts("cannot create tmpfile");
		return(1);
	}

	puts( "Enter your message and end with Ctrl-D");
	printf( "From: %s\n", (ac == 3) ? "Unknown" : ((MASTER) ? "Manager" :
												game.empires[emp].e_name));
	printf( "To: %s\nSubject: ", av[1]);
	fflush(stdout);
	fgets(subj, 80, stdin);

	while( (n = read(0, buf, BUFSIZ)) > 0 )
		write(tmp, buf, (size_t) n);

	puts("Sending telegram..."), fflush(stdout);
	close(tmp);

	for (j = 0; j < NUM_EMPIRES; j++) {

		if (i == -2) sprintf(telefile, "tele_%02d", j);
		else sprintf(telefile, "tele_%02d", i);

		if ((ftmp = fopen(tmpfile, "r")) == NULL) {
			puts("cannot open tmp file");
			return(1);
		}
		if ((ftele = fopen(telefile, "a")) == NULL) {
			puts("cannot open telegram file");
			return(1);
		}

		fprintf(ftele, "From: %s\n", (ac == 3) ? "Unknown" :
				((MASTER) ? "Manager" : game.empires[emp].e_name));
		fprintf(ftele, "To: %s\n", av[1]);
		fprintf(ftele, "Subject: %s\n", subj);

		while( (n = getc(ftmp)) != EOF )
			putc((int) n, ftele);

		putc('\n', ftele);
		fclose(ftmp);
		fclose(ftele);
		if (i > -2) break;
	}
	unlink(tmpfile);
	return(0);
}

#define MAXMSGS 100

struct {
	char m_used;
	long m_offset;
	char m_from[21], m_subject[50];
	int m_lines;
} msgs[MAXMSGS];
int num_msgs;

FILE *fp;
int dot;
long lastseen;
int delta;
int allgone;

jmp_buf tele_env;
char tele_file[10];

void
telegram() {
	sprintf(tele_file, "tele_%02d", emp);
	fp = fopen(tele_file, "r+");
	if (fp == NULL) {
		puts("can't open telegram file");
		return;
	}

	dot = -1;
	delta = 0;
	allgone = 0;

	if (!read_in()) {
		puts("No telegrams.");
		fclose(fp);
		return;
	}
	headers(0);
	telecmds();
	fclose(fp);
}

void
telecmds()
{
	char str1[40], str2[40], *s;
	int c;
	int argnum = 0;
	int cmd;
	void (*sigint)(), (*sigquit)();

	sigint = signal(SIGINT, tele_break);
	sigquit = signal(SIGQUIT, tele_break);

	if (setjmp(tele_env)) {
		putchar('\n');
	}

	for(;;) {
		fputs("& ", stdout);
		fflush(stdout);
		while ((c = getchar()) == ' ');

		if (isalpha(c)) {
			cmd = c;
			c = getchar();
		}
		else {
			cmd = 't';
		}

		while (c == ' ') {
			c = getchar();
		}

		if (isdigit(c)) {
			argnum = 0;
			while (isdigit(c)) {
				argnum = argnum * 10 + c - '0';
				c = getchar();
			}
		}
		else {
			argnum = -1;
		}

		while (c != '\n') {
			c = getchar();
		}

		switch(cmd) {
		case 't':
			if (argnum < 0) {
				if (last_dot()) {
					puts("at end-of-file");
				}
				else {
					inc_dot();
					type(dot);
				}
			}
			else {
				type(argnum);
			}
			break;
		case 'd':
			if (dot == -1) {
				puts("no current message");
				break;
			}
			if (allgone) {
				puts("No more telegrams.");
				break;
			}
			if (argnum < 0) {
				msgs[dot].m_used = 0;
				++delta;
				inc_dot();
			}
			else {
				msgs[argnum].m_used = 0;
				++delta;
				if (argnum == dot)
					inc_dot();
			}
			break;
		case 'h':
			headers((argnum == -1) ? 0 : argnum);
			break;
		case 'q':
			if (delta) {
				FILE *tfp;
				char *tmpfile;
				int n;
				tmpfile = mktemp("tmpXXXXXX");
				tfp = fopen(tmpfile, "w+");
				if (tfp == NULL) {
					puts("cannot make a tmpfile");
					fclose(fp);
					return;
				}
				saveall(tfp);
				fclose(fp);
				fseek(tfp, 0L, 2);
				lastseen = (long) sizeof(long) + ftell(tfp);
				fseek(tfp, 0L, 0);
				fp = fopen(tele_file, "w");
				fwrite(&lastseen, sizeof(long), 1, fp);
				while( fgets(buf, BUFSIZ, tfp) )
					fputs(buf, fp);
				fclose(tfp);
				unlink(tmpfile);
			}
			/* fall through... */
		case 'x':
			signal(SIGINT, sigint);
			signal(SIGQUIT, sigquit);
			return;
		default:
			puts("bizarre command");
		}
	}
}

void
type(int num) {
	if (num < 0 || num >= num_msgs || !msgs[num].m_used) {
		puts("bad message number");
		return;
	}
	fseek(fp, msgs[num].m_offset, 0);
	fgets(buf, BUFSIZ, fp);
	fputs(buf, stdout);
	for(;;) {
		if (!fgets(buf, BUFSIZ, fp)) break;
		if (!strncmp(buf, "From", 4)) break;
		fputs(buf, stdout);
	}
	dot = num;
}

int
read_in() {
	long seeknum;

	num_msgs = 0;

	fread(&lastseen, sizeof(long), 1, fp);

	seeknum = ftell(fp);
	if (!fgets(buf, BUFSIZ, fp))
		return 0;
	for(;;) {
		if (!strncmp(buf, "From: ", 4)) {
			msgs[num_msgs].m_offset = seeknum;
			buf[strlen(buf)-1] = '\0';
			strcpy( msgs[num_msgs].m_from, buf+6);
			fgets(buf, 80, fp);	/* eat up To: field */
			fgets(buf, 80, fp);
			buf[strlen(buf)-1] = '\0';
			strcpy( msgs[num_msgs].m_subject, buf+9);
			msgs[num_msgs].m_lines = 0;
			msgs[num_msgs].m_used = 1;
			++num_msgs;
		}
		else
			++msgs[num_msgs-1].m_lines;
		seeknum = ftell(fp);
		if (!fgets(buf, BUFSIZ, fp)) break;
	}
	return 1;
}

void
headers(int num)
{
	int i;
	if (num_msgs == 0) {
		puts("no telegrams");
		return;
	}
	puts("  No  From\t\t  Lines  Subject");
	if (num >= num_msgs) num = num_msgs - 1;
	if (num < 0) num = 0;
	for(i= (num < 0) ? 0 : num; i < num_msgs; ++i) {
		if (!msgs[i].m_used) continue;
		printf( "%c %2d  %-20s%5d  %s\n", i == dot ? '>' : ' ',
 			i, msgs[i].m_from, msgs[i].m_lines, msgs[i].m_subject);
	}
}

void
tele_break() {
	signal(SIGINT, tele_break);
	signal(SIGQUIT, tele_break);
	longjmp(tele_env, 1);
}

/* is this the last message? */
int
last_dot() {
	int i;
	for( i=dot+1; i < num_msgs; ++i)
		if (msgs[i].m_used) return 0;
	return 1;
}

void
inc_dot() {
	int i;
	for( i=dot+1; i < num_msgs; ++i)
		if (msgs[i].m_used) {
			dot = i;
			return;
		}
	for(i=0; i <= dot; ++i)
		if (msgs[i].m_used) {
			dot = i;
			return;
		}
	puts("No more telegrams");
	allgone = 1;
}

void
saveall(FILE *tfp)
{
	int i;
	for(i=0; i < num_msgs; ++i) {
		if (!msgs[i].m_used) continue;
		fseek(fp, msgs[i].m_offset, 0);
		fgets(buf, BUFSIZ, fp);
		fputs(buf, tfp);
		for(;;) {
			if (!fgets(buf, BUFSIZ, fp)) break;
			if (!strncmp(buf, "From", 4)) break;
			fputs(buf, tfp);
		}
	}
}


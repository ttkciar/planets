/*
 * Planets
 * Original version by Charles Rand
 *
 * 1986-04-07
 * Completely re-written by Chuck Peterson and Keith Reynolds
 * For the U.C. Santa Cruz Game Shell
 *
 * 2018-12-09 Ported to C11 by Bill Karwin
 */

#include "planets.h"

void checklog();
void commands();
int do_init();
void init_tele(int);
int read_tele(int, char);
void start_datafile();
void start_emp();
void start_identity();
void start_mail();
void start_options(int, char **);
void start_system();
void unlock();

char obuf[BUFSIZ];

uid_t master_uid=0, real_uid;
char *config_file = "/etc/planets.conf";
char *playdir = "/var/lib/planets";
char *helpdir = "/var/lib/planets/help";

int
main(int argc, char **argv)
{

    start_system();
    start_options(argc, argv);
    start_identity();
    start_datafile();
    start_emp();
    start_mail();

    commands();
}

void
start_system() {
    if (read(0, "", 0) == -1 || write(1, "", 0) == -1) {
        exit(1);
    }
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    setbuf(stdout, obuf);
    home = getenv("HOME");
    umask(0000);
    seed = getpid();
}

void
start_options(int argc, char **argv) {
    FILE *fp;
    int flag;

    uid = real_uid = getuid();

    if ((fp = fopen(config_file, "r")) != NULL) {
        char *param="", *value="";
        while (fscanf(fp, "%s = %s", param, value) > 0) {
            if (!strcmp(param, "master_uid")) {
                game_config.master_uid = (uid_t) strtol(value, NULL, 10);
            } else if (!strcmp(param, "playdir")) {
                game_config.playdir = strdup(value);
            }
        }
        fclose(fp);
    }

    while ((flag = getopt(argc, argv, "d:u:")) != -1) {
        switch (flag) {
            case 'd':
                playdir = optarg;
                break;
            case 'u':
                uid = (uid_t) strtol(optarg, NULL, 10);
                break;
            default:
                exit(1);
        }
    }
    argc -= optind;
    argv += optind;
}

void
start_identity() {
    if (MASTER) {
        puts("You are in administrator mode");
        if (uid != master_uid) {
            printf("Uid set to %d\n", uid);
            fflush(stdout);
        }
    }
    setgid(getgid());
}

void
start_datafile() {
    if (chdir(playdir)) {
        perror(playdir);
        exit(1);
    }
    fd = open(DATA_FILE, 2);
    if (fd < 0) {
        if (MASTER) {
            unlock();
            close(creat(DATA_FILE, 0660));
            fd = open(DATA_FILE, 2);
            if (fd < 0) {
                puts("Cannot create datafile");
                fflush(stdout);
                exit(0);
            }
            puts("Creating Data File");
            fflush(stdout);
            do_init();
        }
        else {
            puts("Cannot open data file");
            fflush(stdout);
            exit(0);
        }
    }
    lseek(fd, 0L, 0);
    read(fd, &game, sizeof(game));
}

void
start_emp() {
    int i;
    emp = -1;
    for(i=0; i < NUM_EMPIRES; ++i) {
        if (game.empires[i].e_uid == uid) {
            emp = i;
            break;
        }
    }
}

void
start_mail() {
    if (uid != master_uid || emp != -1) {
        read_tele(emp, 0);
        checklog();
    }
}

/* from rogue */
#define RN (((seed = seed*11109+13849) & 0x7fff) >> 1)

int
rnd(int n) {
    return (int) (n <= 0 ? 0 : RN % n);
}

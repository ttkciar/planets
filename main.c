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
    char *config_file;
    int exit_value = 1;

    uid = real_uid = getuid();

    config_file = getenv("PLANETS_CONFIG");
    if (!config_file) {
        config_file = DEFAULT_CONFIG;
    }
    game_config.playdir = DEFAULT_PLAYDIR;
    game_config.helpdir = DEFAULT_HELPDIR;

    if ((fp = fopen(config_file, "r")) != NULL) {
        char param[20], value[100];
        while (fscanf(fp, "%s = %s", param, value) > 0) {
            if (!strcmp(param, "master_uid")) {
                game_config.master_uid = (uid_t) strtol(value, NULL, 10);
            } else if (!strcmp(param, "playdir")) {
                game_config.playdir = strdup(value);
            } else if (!strcmp(param, "helpdir")) {
                game_config.helpdir = strdup(value);
            }
        }
        fclose(fp);
    }

    while (1) {
        static struct option long_options[] = {
                {"help", no_argument, 0, 'h'},
                {"helpdir", required_argument, 0, 'H'},
                {"master-uid", required_argument, 0, 'm'},
                {"playdir", required_argument, 0, 'p'},
                {0, 0, 0, 0}
        };
        int option_index = 0;
        flag = getopt_long(argc, argv, "hHmp", long_options, &option_index);
        if (flag == -1) {
            break;
        }
        switch (flag) {
            case 0:
                break;
            case 'H':
                game_config.helpdir = optarg;
                break;
            case 'm':
                game_config.master_uid = (uid_t) strtol(optarg, NULL, 10);
                break;
            case 'p':
                game_config.playdir = optarg;
                break;
            case 'h':
                exit_value = 0;
            case '?':
                printf("Get help!\n");
                fflush(stdout);
                exit(exit_value);
            default:
                abort();
        }
    }
    game_config.is_master = (game_config.master_uid == real_uid);
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
    if (chdir(game_config.playdir)) {
        perror(game_config.playdir);
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

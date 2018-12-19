#include <fcntl.h>
#include <zconf.h>
#include "planets.h"

static void (*sigint)();

int
lock(int n) {
	int lfd;
	int num = n?4:0;
	sigint = signal(SIGINT, SIG_IGN);

	for(;;) {
		lfd = creat("LOCK", 0000);
		if (lfd == -1) {
			if (num--) {
				puts("busy...");
				fflush(stdout);
				sleep(1);
			}
			else {
				puts("Aborted.");
				fflush(stdout);
				signal(SIGINT, sigint);
				return 0;
			}
		}
		else {
			close(lfd);
			return 1;
		}
	}
}

void
unlock() {
	unlink("LOCK");
	signal(SIGINT, sigint);
}

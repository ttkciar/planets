SRC = build.c cmds.c command.c conflict.c direct.c init.c lock.c log.c\
		main.c make.c misc.c plot.c print.c range.c\
		score.c telegram.c turn.c universe.c update.c

OBJ = build.o cmds.o command.o conflict.o direct.o init.o lock.o log.o\
		main.o make.o misc.o plot.o print.o range.o\
		score.o telegram.o turn.o universe.o update.o

CFLAGS=-O
LDFLAGS=-X -i

a.out : $(OBJ)
	ld /lib/crt0.o $(LDFLAGS) $(OBJ) -lm -ltermlib -lucsc -lc
	@echo "Done"

$(OBJ): planets.h

clean:
	rm -f $(OBJ) a.out

shar:
	../../bin/shar $(SRC) planets.h makefile > planets.shar

tar:
	tar -cf planets.tar $(SRC) planets.h makefile

size:
	wc $(SRC) planets.h

install:
	-mv /usr/games/type2/planets /usr/games/type2/planets.$$$$
	@rm -f /usr/games/type2/planets*
	-mv a.out /usr/games/type2/planets
	chmod 701 /usr/games/type2/planets
	ls -l /usr/games/type2/planets*

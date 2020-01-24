CC=		cc

CFLAGS=		-Wall -Wextra -O3 -ffast-math -std=gnu99 -march=native -mtune=native -Wshadow -Wpointer-arith -Wstrict-prototypes -Wmissing-prototypes
DFLAGS=

#CFLAGS=		-Wall -Wextra -O0 -ggdb -std=gnu99
#DFLAGS=		-DPRGDEBUG

PROG=		cma

SOLVER=		hash_iset.c hash_map.c cma.c

SOLVEROBJS=	$(SOLVER:.c=.o)

.SUFFIXES:.c .o

all:		$(PROG)

$(PROG):	$(SOLVEROBJS)
		$(CC) $(CFLAGS) -o $(PROG) $(SOLVEROBJS)

.c.o:
	$(CC) -c $(CFLAGS) $(DFLAGS) $< -o $@

.PHONY : clean

clean :
	rm -rf $(PROG) *.o *.dSYM *~

SWEEPNYC_C_FLAGS=-c -O2 -Wall -Wextra -Waggregate-return -Wcast-align -Wcast-qual -Wconversion -Wformat=2 -Winline -Wlong-long -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Wno-import -Wpointer-arith -Wredundant-decls -Wreturn-type -Wshadow -Wstrict-prototypes -Wswitch -Wwrite-strings

sweepnyc.exe: sweepnyc.o
	gcc -o sweepnyc.exe sweepnyc.o

sweepnyc.o: sweepnyc.c sweepnyc.make
	gcc ${SWEEPNYC_C_FLAGS} -o sweepnyc.o sweepnyc.c

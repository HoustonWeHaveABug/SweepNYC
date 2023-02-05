SWEEPNYC_DEBUG_C_FLAGS=-c -g -std=c89 -Wpedantic -Wall -Wextra -Waggregate-return -Wcast-align -Wcast-qual -Wconversion -Wformat=2 -Winline -Wlong-long -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Wpointer-arith -Wredundant-decls -Wshadow -Wstrict-prototypes -Wwrite-strings -Wswitch-default -Wswitch-enum -Wbad-function-cast -Wstrict-overflow=5 -Wundef -Wlogical-op -Wfloat-equal -Wold-style-definition

sweepnyc_debug: sweepnyc_debug.o
	gcc -g -o sweepnyc_debug sweepnyc_debug.o

sweepnyc_debug.o: sweepnyc.c sweepnyc_debug.make
	gcc ${SWEEPNYC_DEBUG_C_FLAGS} -o sweepnyc_debug.o sweepnyc.c

clean:
	rm -f sweepnyc_debug sweepnyc_debug.o

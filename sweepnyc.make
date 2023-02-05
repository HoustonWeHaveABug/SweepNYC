SWEEPNYC_C_FLAGS=-c -O2 -std=c89 -Wpedantic -Wall -Wextra -Waggregate-return -Wcast-align -Wcast-qual -Wconversion -Wformat=2 -Winline -Wlong-long -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Wpointer-arith -Wredundant-decls -Wshadow -Wstrict-prototypes -Wwrite-strings -Wswitch-default -Wswitch-enum -Wbad-function-cast -Wstrict-overflow=5 -Wundef -Wlogical-op -Wfloat-equal -Wold-style-definition

sweepnyc: sweepnyc.o
	gcc -o sweepnyc sweepnyc.o

sweepnyc.o: sweepnyc.c sweepnyc.make
	gcc ${SWEEPNYC_C_FLAGS} -o sweepnyc.o sweepnyc.c

clean:
	rm -f sweepnyc sweepnyc.o


CFLAGS=-Wall -g -std=c11
HEADERS=$(wildcard *.h)

all : simul

simul: cpu.o systeme.o asm.o simul.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -vf *.o simul


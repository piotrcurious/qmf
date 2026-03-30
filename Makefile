CC = gcc
CFLAGS = -Wall -Wextra -O2
LIBS = -lm

all: test_qmf filter-main

qmf.o: qmf.c qmf.h
	$(CC) $(CFLAGS) -c qmf.c

test_qmf: test_qmf.c qmf.o
	$(CC) $(CFLAGS) -o test_qmf test_qmf.c qmf.o $(LIBS)

filter-main: filter-main.c qmf.o
	$(CC) $(CFLAGS) -o filter-main filter-main.c qmf.o $(LIBS)

clean:
	rm -f *.o test_qmf filter-main

CC = gcc
CFLAGS = -Wall -Wextra -O2
LIBS = -lm

TARGETS = test_qmf filter-main comprehensive_test gen_data

all: $(TARGETS)

qmf.o: qmf.c qmf.h
	$(CC) $(CFLAGS) -c qmf.c

test_qmf: test_qmf.c qmf.o
	$(CC) $(CFLAGS) -o test_qmf test_qmf.c qmf.o $(LIBS)

filter-main: filter-main.c qmf.o
	$(CC) $(CFLAGS) -o filter-main filter-main.c qmf.o $(LIBS)

comprehensive_test: comprehensive_test.c qmf.o
	$(CC) $(CFLAGS) -o comprehensive_test comprehensive_test.c qmf.o $(LIBS)

gen_data: gen_data.c qmf.o
	$(CC) $(CFLAGS) -o gen_data gen_data.c qmf.o $(LIBS)

visualize: gen_data
	./gen_data
	python3 visualize.py

clean:
	rm -f *.o $(TARGETS) sweep.csv recon.csv sweep.svg recon.svg filter-main.log

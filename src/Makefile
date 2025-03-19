CC=gcc
PROGRAM=proyecto1

all: main.o split.o util.o
	$(CC) -o $(PROGRAM) main.o split.o util.o

split.o: split.c split.h util.h
	$(CC) -c -o split.o split.c

main.o: main.c split.h util.h
	$(CC) -c -o main.o main.c

util.o: util.c util.h
	$(CC) -c -o util.o util.c

clean:
	-rm -rf *.o $(PROGRAM) $(PROGRAM).exe

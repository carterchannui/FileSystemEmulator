CC = gcc
CFLAGS = -Wall -std=c99 -pedantic -g
MAIN = main
OBJS = main.o main_tests.o

all : $(MAIN)

$(MAIN) : $(OBJS) main_tests.h
	$(CC) $(CFLAGS) -o $(MAIN) $(OBJS)

main.o : main.c main_tests.h
	$(CC) $(CFLAGS) -c main.c

main_tests.o : main_tests.c checkit.h
	$(CC) $(CFLAGS) -c main_tests.c

clean :
	rm -f *.o $(MAIN) core
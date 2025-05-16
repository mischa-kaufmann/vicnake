CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g
LIBS = -lncurses

vicnake: vicnake.c
	$(CC) $(CFLAGS) vicnake.c -o vicnake $(LIBS)

clean:
	rm -f vicnake
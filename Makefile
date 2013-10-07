#Nate Gillard
#cs457 P2
#awget: Anonymous wget. Stepping stone servers and awget request.
CC = g++ 
CFLAGS = -Wall -Wextra -ansi -gstabs
OBJS = ss.o awget.o

all: $(OBJS)
	$(CC) $(CFLAGS) ss.o -o ss
	$(CC) $(CFLAGS) awget.o -o awget

%.o: %.cc
	$(CC) $(CFLAGS) -c $<

clean:
	rm -v ss awget *.o 

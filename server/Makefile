#example compile
#gcc enroll.c -o enroll -L../libfprint/.libs -lfprint

#Compiler
CC=gcc

#compiler flags
CXXFLAGS= -g -Wall -Wextra

#libs to include
LDFLAGS=-lfprint arduino-serial-lib/arduino-serial-lib.c

#change this
default: test

arduino-lock:
	$(CC) -c $(CXXFLAGS) $(LDFLAGS) $@.c

test: arduino-lock
	$(CC) -o $@ $(CXXFLAGS) arduino-lock.o $(LDFLAGS) $@.c

clean:
	rm -rf *.o test


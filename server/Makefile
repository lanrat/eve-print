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

arduino_lock:
	$(CC) -c $(CXXFLAGS) $(LDFLAGS) $@.c

test: arduino_lock
	$(CC) -o $@ $(CXXFLAGS) arduino_lock.o $(LDFLAGS) $@.c

clean:
	rm -rf *.o test


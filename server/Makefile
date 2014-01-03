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

finger_auth:
	$(CC) -c $(CXXFLAGS) $(LDFLAGS) $@.c

test: arduino_lock finger_auth
	$(CC) -o $@ $(CXXFLAGS) arduino_lock.o finger_auth.o $(LDFLAGS) $@.c

clean:
	rm -rf *.o test


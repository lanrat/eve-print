#ifndef __ARDUINO_LOCK_H
#define __ARDUINO_LOCK_H

#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include "arduino-serial-lib/arduino-serial-lib.h"

#define ARDUINO_SEARCH_PATH "/dev/serial/by-id/"
#define ARDUINO_SEARCH_ID "Arduino"
#define ARDUINO_BAUDRATE 9600
#define ARDUINO_PATH_SIZE 512
#define ARDUINO_MAX_ATTEMPTS 2

#define false 0
#define true 1
typedef int bool;

bool arduino_force_flush;

char * arduino_find();

void arduino_connect(char* port);
void arduino_close();

void arduino_unlock();;
void arduino_blink();

#endif

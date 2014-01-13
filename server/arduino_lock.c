#include "arduino_lock.h"

int fd = -1;
bool auto_find = false;
char auto_device_path[ARDUINO_PATH_SIZE];
bool arduino_force_flush = false;

char * arduino_find()
{
    auto_find = true;
    DIR * d;
    struct dirent *dir;
    char path[ARDUINO_PATH_SIZE];

    d = opendir(ARDUINO_SEARCH_PATH);
    if (d) {
        while ((dir = readdir(d)) != NULL)
        {
            if (strstr(dir->d_name,ARDUINO_SEARCH_ID))
            {
                closedir(d);
                //get the full path
                snprintf(path,sizeof(path),"%s/%s",ARDUINO_SEARCH_PATH,dir->d_name);
                //get the link destination
                realpath(path,auto_device_path);
                return auto_device_path;
            }
        }
        closedir(d);
    }
    return NULL;
}

void arduino_connect(char* port)
{
    if (port == NULL)
    {
        port = arduino_find();
    }
    fd = serialport_init(port, ARDUINO_BAUDRATE);
    if (fd == -1)
    {
        fprintf(stderr,"Couldn't open port %s\n",port);
    }
    sleep(1);
}


void arduino_send_char_a(char c, int attempts)
{
    int rc;
    rc = serialport_writebyte(fd, (uint8_t)c);

    if (rc == -1){
        fprintf(stderr,"Error writing to arduino\n");
        //attempt reconnect
        if (auto_find && attempts < ARDUINO_MAX_ATTEMPTS) {
            arduino_connect(NULL);
            arduino_send_char_a(c,attempts+1);
        }
    }else{
        if (arduino_force_flush) serialport_flush(fd);
    }
}


void arduino_send_char(char c){
    arduino_send_char_a(c,0);
}

//unlock the door
void arduino_unlock()
{
    arduino_send_char('g');
}


//blink the red LED
void arduino_blink()
{
    arduino_send_char('b');
}


void arduino_close()
{
    if (fd != -1) {
        serialport_close(fd);
        fd = -1;
    }
}


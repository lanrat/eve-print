/*
 * Built from code provided in libfprint/examples
 */
#include <stdio.h>
#include <unistd.h> //for sleep
#include <time.h>

#include "arduino_lock.h"
#include "finger_auth.h"


FILE* logstream;;
struct fp_dev* dev;
struct user_prints users;

void enrollFinger()
{
    char name[NAME_SIZE];
    char filepath[BUFSIZ];
    FILE* outFile = NULL;
    bool fileAvaible = false;
    size_t name_end;
    struct fp_print_data *enrolled_print = NULL;
    dev = fp_connect();

    printf("You are about to add a new fingerprint to the database. Doing so "
            "will require a restart of the verification program.\n");

    printf("Please enter a name for the print. The name of the person owning "
            "the print is usually a good idea.\n");


    while (!fileAvaible)
    {
        printf("Name: ");
        if ( fgets(name, NAME_SIZE, stdin) == NULL  ){
            return;
        }
        name[NAME_SIZE-1] = '\0'; //NULL terminate
        name_end = strnlen(name,NAME_SIZE);
        if (name_end < 3) {
            printf("Name must be at least 2 chars\n");
            continue;
        }
        if (name_end > 1)
        {
            name[name_end-1] = '\0'; //repalce newline with NULL
        }
        (void)snprintf(filepath, BUFSIZ, "%s/%s.%s", PATH, name, EXT);
        if (file_exists(filepath))
        {
            printf("%s already exists.\n",name);
        }else{
            fileAvaible = true;
        }
    }

    
    printf("You will need to successfully scan your finger %d times to "
            "complete the process.\n", fp_dev_get_nr_enroll_stages(dev)*2);
    

    if (!enroll(dev,&enrolled_print)) {
        fp_print_data_free(enrolled_print);
        fp_disconnect(dev);
        return; 
    }

    printf("Verify that the read was good\n");
    if (!verify(dev,enrolled_print)) {
        fp_print_data_free(enrolled_print);
        fp_disconnect(dev);
        return;
    }

    //get a buffer containing the raw data to save to disk
    unsigned char *buf;
    size_t len;

    len = fp_print_data_get_data(enrolled_print, &buf);
    if (!len) {
        fprintf(stderr,"Unable to get data buffer for print");
    }

    fp_print_data_free(enrolled_print);
    fp_disconnect(dev);
    
    //save enrolled_print
    outFile = fopen(filepath,"wb");
    if (outFile == NULL)
    {
        fprintf(stderr,"Unable to open %s for writing.\n",filepath);
        return;
    }
    if (fwrite(buf, len, 1, outFile) != 1)
    {
        fprintf(stderr,"Error while writing to %s.\n",filepath);
        free(buf);
        fclose(outFile);
        return;
    }

    printf("Enrollment completed!\n\n");
    free(buf);
    fclose(outFile);
}



void youShallPass()
{
    arduino_unlock();
}

void youShallNotPass()
{
    arduino_blink();
}

void auth_exit()
{
    freePrints(&users);
    arduino_close();
    fp_disconnect(dev);
}

void auth(char* arduino_port)
{
    dev = fp_connect();
    int result;

    if (fp_dev_supports_identification(dev) == 0){
        fprintf(stderr,"Device does not suport identification!\n");
        return;
    }

    //load all prints from PATH
    loadPrints(&users);

    if (users.length < 1) {
        fprintf(stderr,"No prints loaded!\n");
        return;
    }

    arduino_connect(arduino_port);

    //time code
    time_t now;
    char tBuffer[25];
    struct tm* tm_info;
    time(&now);
    tm_info = localtime(&now);
    strftime(tBuffer, 25, "%d/%b/%Y:%H:%M:%S", tm_info);

    fprintf(logstream,"[%s] Starting Auth with %zu prints\n",tBuffer,users.length);
    fflush(logstream);

    do
    {
        result = identify(dev, &users);
        
        time(&now);
        tm_info = localtime(&now);
        strftime(tBuffer, 25, "%d/%b/%Y:%H:%M:%S", tm_info);

        if (result > -1){
            fprintf(logstream,"[%s] Valid Print: %s\n",tBuffer,users.names[result]);
            youShallPass();
        }else{
            fprintf(logstream,"[%s] Invalid Print\n",tBuffer);
            youShallNotPass();
        }
        fflush(logstream);
    }while(true);

    auth_exit();
}

void printUsage(char* self)
{
    printf("Usage:\n");
    printf("%s -h\t\t\tDisplay this message\n",self);
    printf("%s -e\t\t\tEnroll a new print\n",self);
    printf("%s -p ArduinoPort\t\tScan for prints\n",self);
    printf("%s -p ArduinoPort -g\tUnlock the Door\n",self);
    printf("%s -p ArduinoPort -b\tBlink the LED\n",self);
    printf("Order of arguments matter!\n");
}


void testUnlock(char* port)
{
    arduino_connect(port);
    sleep(1);
    arduino_unlock();
    sleep(5);
    arduino_close();
}

void testBlink(char* port)
{
    arduino_connect(port);
    sleep(1);
    arduino_blink();
    sleep(1);
    arduino_close();
}


int main(int argc, char** argv)
{
    int c;
    char * arduino_port = NULL;
    logstream = stdout;
    create_dir(PATH);
    if (argc > 1)
    {
        while ((c = getopt(argc, argv, "ehp:gb")) != -1)
        {
            switch (c)
            {
                case 'e':
                    enrollFinger();
                    return 0;
                case 'p':
                    arduino_port = optarg;
                    break;
                case 'g':
                    testUnlock(arduino_port);
                    return 0;
                    break;
                case 'b':
                    testBlink(arduino_port);
                    return 0;
                    break;
                case '?':
                    printf("Unrecognized option -%c\n",optopt);
                case 'h':
                    printUsage(argv[0]);
                    return 0;
            }
        }
        if (arduino_port != NULL) {
            auth(arduino_port);
        }else {
            fprintf(stderr,"Missing port\n");
        }
    }else{
        printUsage(argv[0]);
    }
    return 0;
}


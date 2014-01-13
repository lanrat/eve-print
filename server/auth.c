#include <stdio.h>
#include <unistd.h> //for getopt
#include <time.h> //for sleep
#include <signal.h>

#include "arduino_lock.h"
#include "finger_auth.h"

#define LOG_TIMESTAMP "%d/%b/%Y %H:%M:%S"

FILE *logstream;
struct fp_dev* dev;
struct user_prints users;

void enrollFinger(char* prints_path)
{
    char name[PRINT_NAME_SIZE];
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
        if ( fgets(name, PRINT_NAME_SIZE, stdin) == NULL  ){
            return;
        }
        name[PRINT_NAME_SIZE-1] = '\0'; //NULL terminate
        name_end = strnlen(name,PRINT_NAME_SIZE);
        if (name_end < 3) {
            printf("Name must be at least 2 chars\n");
            continue;
        }
        if (name_end > 1)
        {
            name[name_end-1] = '\0'; //repalce newline with NULL
        }
        (void)snprintf(filepath, BUFSIZ, "%s/%s.%s", prints_path, name, PRINT_EXT);
        if (file_exists(filepath))
        {
            printf("%s already exists.\n",name);
        }else{
            fileAvaible = true;
        }
    }

    bool verified = false;
    while (!verified)
    {
        printf("You will need to successfully scan your finger %d times to "
                "complete the process.\n", fp_dev_get_nr_enroll_stages(dev)*2);

        if (!enroll(dev,&enrolled_print)) {
            continue;
        }

        printf("Verify that the read was good\n");
        verified = verify(dev,enrolled_print);
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
    //fp_disconnect(dev); //errors for some reason
}

void auth_signal(int signum)
{
    time_t now;
    char tBuffer[25];
    struct tm* tm_info;
    time(&now);
    tm_info = localtime(&now);
    strftime(tBuffer, 25, LOG_TIMESTAMP, tm_info);

    fprintf(logstream,"[%s] Exiting with signal %d\n",tBuffer,signum);
    fflush(logstream);

    auth_exit();
    exit(signum);
}

void auth(char* prints_path, char* arduino_port)
{
    dev = fp_connect();
    int result;

    if (fp_dev_supports_identification(dev) == 0){
        fprintf(stderr,"Device does not suport identification!\n");
        return;
    }

    //load all prints from prints_path
    loadPrints(prints_path, &users);

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
    strftime(tBuffer, 25, LOG_TIMESTAMP, tm_info);

    fprintf(logstream,"[%s] Starting Auth with %zu prints\n",tBuffer,users.length);
    fflush(logstream);

    signal(SIGINT, auth_signal); //Ctrl-C
    signal(SIGTERM, auth_signal); //kill command

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

void printUsage(char* me)
{
    printf("Usage: %s [options] operand\n",me);
    printf("Operands:\n");
    printf("\tenroll\tScans and saves a new finger\n");
    printf("\tauth\tRuns the authentication for the lock\n");
    printf("\tunlock\tUnlocks the door\n");
    printf("\tblink\tBlinks the red LED\n");
    printf("Options:\n");
    printf("\t-h\tDisplay this message\n");
    printf("\t-f\tprints_folder\tDefault: %s\n",PRINT_DEFAULT_PATH);
    printf("\t-p\tarduino_port\tDefault: Automatic\n");
}



int main(int argc, char** argv)
{
    int c;
    char* arduino_port = NULL;
    logstream = stdout;
    char *prints_path = PRINT_DEFAULT_PATH;

    //parse args
    while ((c = getopt(argc, argv, "hf:p:")) != -1)
    {
        switch (c)
        {
            case 'p':
                arduino_port = optarg;
                break;
            case 'f':
                prints_path = optarg;
                break;
            case '?':
            case 'h':
                printUsage(argv[0]);
                return 0;
        }
    }

    if (optind >= argc) {
        printUsage(argv[0]);
        return 0;
    }

    if (arduino_port == NULL) {
        //autodetect arduino
        arduino_port = arduino_find();
        printf("Using Arduino: %s\n",arduino_port);
    }

    //parse operand
    if (strcmp(argv[optind], "auth")==0){
        auth(prints_path, arduino_port);
    }else if (strcmp(argv[optind], "enroll")==0){
        enrollFinger(prints_path);
    }else if (strcmp(argv[optind], "unlock")==0){
        testUnlock(arduino_port);
    }else if (strcmp(argv[optind], "blink")==0){
        testBlink(arduino_port);
    }else{
        fprintf(stderr,"Unknown operand %s\n\n",argv[optind]);
        printUsage(argv[0]);
    }

    return 0;
}


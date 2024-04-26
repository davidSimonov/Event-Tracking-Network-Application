/*
    David Simonov
    das892
    11301273
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>

#define BUFFER_SIZE 128 /* Buffer size for the request */
#define STRING_SIZE 20 /* Maximum allowed string for the arguments */

char request[BUFFER_SIZE] = {0};
int sockfd;

/* enum for all the different commands */
typedef enum {
    ADD,
    REMOVE,
    UPDATE,
    GET,
    GETALL,
    UNKNOWN
} CommandType;

/*
    Takes the command argument
    Returns the command type entered, UNKNOWN if command isnt recognised
*/
CommandType getCommandType(const char *command) {
    if (strcmp(command, "add") == 0) return ADD;
    if (strcmp(command, "remove") == 0) return REMOVE;
    if (strcmp(command, "update") == 0) return UPDATE;
    if (strcmp(command, "get") == 0) return GET;
    if (strcmp(command, "getall") == 0) return GETALL;
    return UNKNOWN;
}

/*
    Takes the date argument
    Returns 1 if its is valid, -1 if it cant be processesd
*/
int validDate(const char *date) {
    int i;
    char dd[3], mm[3], yyyy[5];
    int day, month, year;
    
    if (strlen(date) != 10)
        return -1;

    for (i = 0; i < 10; i++) {
        if ((i == 2 || i == 5) && date[i] != ':')
            return -1;
        else if (i != 2 && i != 5 && !isdigit(date[i]))
            return -1;
    }

    strncpy(dd, date, 2);
    dd[2] = '\0';
    strncpy(mm, date + 3, 2);
    mm[2] = '\0';
    strncpy(yyyy, date + 6, 4);
    yyyy[4] = '\0';

    day = atoi(dd);
    month = atoi(mm);
    year = atoi(yyyy);

    if ((day < 1 || day > 31) || (month < 1 || month > 12) || (year < 2024))
        return -1;
    
    return 1;
}

/*
    Takes the time argument
    Returns 1 if its is valid, -1 if it cant be processesd
*/
int validTime(const char *time) {
    int i;
    char hh[3], mm[3];
    int hours, minutes;
    
    if (strlen(time) != 4)
        return -1;

    for (i = 0; i < 4; i++) {
        if (!isdigit(time[i]))
            return -1;
    }

    strncpy(hh, time, 2);
    hh[2] = '\0';
    strncpy(mm, time + 2, 2);
    mm[2] = '\0';

    hours = atoi(hh);
    minutes = atoi(mm);

    if ((hours < 0 || hours > 24) || (minutes < 0 || minutes > 59))
        return -1;

    return 1;
}

/*
    Takes the title/username argument
    Returns 1 if its is valid, -1 if it cant be processesd
*/
int validString(const char *title) {
    
    if (strlen(title) < 1 || strlen(title) > STRING_SIZE)
        return 0;

    if (strchr(title, ' ') != NULL)
        return 0;

    return 1;
}

/*
    Takes the protocol buffer to be sent to the server
    Returns 0 on success, -1 on failure
*/
int sendRequest(char* buffer) {

    /* Send request to server */
    ssize_t numBytesRcvd;
    if (send(sockfd, request, strlen(request), 0) < 0) {
        perror("Error sending request");
        return -1;
    }

    memset(buffer, 0, BUFFER_SIZE);
    
    /* Receive response from server */
    numBytesRcvd = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
    if (numBytesRcvd < 0) {
        perror("Error receiving response");
    } else if (numBytesRcvd == 0) {
        printf("Connection closed by server\n");
    } else {
        buffer[numBytesRcvd] = '\0';
        return 0;
    }
    return -1;
}

/*
    Builds the protocol buffer for "add" to send to the server
    Prints the response
*/
void addCMD(char* user, char* date, char* start, char* end, char* event) {
    char buffer[BUFFER_SIZE];
    printf("ADD validated, sending to server\n"); /* TEST */

    snprintf(request, BUFFER_SIZE, "%s ADD %s %s %s %s", 
            user, date, start, end, event);

    if (sendRequest(buffer) == 0) printf("Server response: %s\n", buffer);
}

/*
    Builds the protocol buffer for "remove" to send to the server
    Prints the response
*/
void removeCMD(char* user, char* date, char* start) {
    char buffer[BUFFER_SIZE];
    printf("REMOVE validated, sending to server\n"); /* TEST */

    snprintf(request, BUFFER_SIZE, "%s REMOVE %s %s",
            user, date, start);

    if (sendRequest(buffer) == 0) printf("Server response: %s\n", buffer);
}

/*
    Builds the protocol buffer for "update" to send to the server
    Prints the response
*/
void updateCMD(char* user, char* date, char* start, char* end, char* event) {
    char buffer[BUFFER_SIZE];
    printf("UPDATE validated, sending to server\n"); /* TEST */

    snprintf(request, BUFFER_SIZE, "%s UPDATE %s %s %s %s", 
            user, date, start, end, event);

    if (sendRequest(buffer) == 0) printf("Server response: %s\n", buffer);
}

/*
    Builds the protocol buffer for "get" to send to the server
    Prints the response
*/
void getCMD(char* user, char* date, char* start) {
    char buffer[BUFFER_SIZE];
    printf("GET validated, sending to server\n"); /* TEST */

    if (start) {
        snprintf(request, BUFFER_SIZE, "%s GET %s %s",
        user, date, start);
    } else {
        snprintf(request, BUFFER_SIZE, "%s GET %s",
        user, date);
    }

    if (sendRequest(buffer) == 0) printf("Server response: %s\n", buffer);
}

/*
    Builds the protocol buffer for "getevent" to send to the server
    getevent is a private request, 
    used by "getall" for the purpose of getting an event at index eventNumber
*/
int getevent(char* user, char* buffer, int eventNumber) {
    snprintf(request, BUFFER_SIZE, "%s GETEVENT %d", user, eventNumber);
    printf("Calling GETEVENT for event %d\n", eventNumber); /* TEST */

    return sendRequest(buffer);
}

/*
    Builds the protocol buffer for "getall" to send to the server
    Utilizes "getevent" for getting each event with a 2sec delay
    Prints the response
*/
void getallCMD(char* user) {
    char buffer[BUFFER_SIZE];
    int count;
    int i;
    printf("GETALL validated, sending to server\n"); /* TEST */

    snprintf(request, BUFFER_SIZE, "%s GETALL", user);

    if (sendRequest(buffer) == 0 && strncmp(buffer, "COUNT:", 6) == 0) {
        count = atoi(buffer + 6);

        for (i = 1; i <= count; ++i) {
            sleep(2);
            if (getevent(user, buffer, i) == 0) 
                printf("Server response: %s\n", buffer);
        }
    } else printf("Getting event count failed\n");
}

int main(int argc, char *argv[]) {

    struct sockaddr_in server_addr;
    
    char *server_ip;
    int port;
    char user[STRING_SIZE] = {0};
    char *command;

    /* Check if correct number of arguments are entered */
    if (argc < 5) {
        fprintf(stderr, "Usage: %s <hostname> <port> <username 1-20> <command>"\
                " [additional arguments]\n", argv[0]);
        return 1;
    }

    /* Check if username is valid */
    if (!validString(argv[3])) {
        fprintf(stderr, "Username must be less than 20 characters and " \
            "cannot contain whitespace\n");
        return 1;
    }


    server_ip = argv[1];
    port = atoi(argv[2]);
    strncpy(user, argv[3], STRING_SIZE);
    command = argv[4];

    /* Create socket */ 
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error creating socket");
        return 1;
    }

    /* Define server address */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);

    /* Connect to the server */
    if (connect(sockfd, (struct sockaddr *)&server_addr, 
        sizeof(server_addr)) < 0) {
        perror("Error connecting to server");
        close(sockfd);
        return 1;
    } else printf("Connected to server.\n");

    /* 
    Switch case for each command type
    Prints the correct command usage if a command is used incorrectly
    */
    switch (getCommandType(command)) {
    case ADD:
    printf("Processing ADD command\n"); /* TEST */
        if (argc == 9 &&
            validDate(argv[5]) &&
            validTime(argv[6]) &&
            validTime(argv[7]) &&
            validString(argv[8]) &&
            atoi(argv[6]) < atoi(argv[7])) 
        {
            addCMD(user, argv[5], argv[6], argv[7], argv[8]);
        } 
        else {
            fprintf(stderr, "Usage: add <date> <start-time> <end-time> <Event>"\
                            "\ndate -> dd:mm:yyyy\n"\
                            "time -> hhmm (start must be before end)\n"\
                            "event -> title (1-20 char no whitespace)\n");
            close(sockfd);
            return 1;
        } break;

    case REMOVE:
    printf("Processing REMOVE command\n"); /* TEST */
        if (argc == 7 &&
            validDate(argv[5]) &&
            validTime(argv[6])) 
        {
            removeCMD(user, argv[5], argv[6]);
        } 
        else {
            fprintf(stderr, "Usage: remove <date> <start-time>\n"\
                            "date -> dd:mm:yyyy\n"\
                            "time -> hhmm (military time)\n");
            close(sockfd);
            return 1;
        } break;

    case UPDATE:
    printf("Processing UPDATE command\n"); /* TEST */
        if (argc == 9 &&
            validDate(argv[5]) &&
            validTime(argv[6]) &&
            validTime(argv[7]) &&
            validString(argv[8]) &&
            atoi(argv[6]) < atoi(argv[7])) 
        {
            updateCMD(user, argv[5], argv[6], argv[7], argv[8]);
        } 
        else {
        fprintf(stderr,"Usage: update <date> <start-time> <end-time> <Event>\n"\
                            "date -> dd:mm:yyyy\n"\
                            "time -> hhmm (start must be before end)\n"\
                            "event -> title (1-20 char no whitespace)\n");
            close(sockfd);
            return 1;
        } break;

    case GET:
    printf("Processing GET command\n"); /* TEST */
        if ((argc == 6 && validDate(argv[5])) ||
            (argc == 7 && validDate(argv[5]) && validTime(argv[6])))
        {
            if (argc == 6) {
                getCMD(user, argv[5], NULL);
            } else {
                getCMD(user, argv[5], argv[6]);
            }
        } 
        else {
            fprintf(stderr, "Usage: get <date> <start-time>\n"\
                            "date -> dd:mm:yyyy\n"\
                            "time -> hhmm (military time) *optional*\n");
            close(sockfd);
            return 1;
        } break;

    case GETALL:
    printf("Processing GETALL command\n"); /* TEST */
        if (argc == 5) 
        {
            getallCMD(user);
        } 
        else {
            fprintf(stderr, "Usage: getall\n");
            close(sockfd);
            return 1;
        } break;

    default:
        fprintf(stderr, "Invalid command\n"\
            "List of valid commands: add, remove, update, get, getall\n");
        close(sockfd);
        return 1;
    }

    /* Close socket */
    close(sockfd);
    printf("Disconnected from server.\n");

    return 0;
}
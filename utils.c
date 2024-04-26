/*
    David Simonov
    das892
    11301273
*/

#include <utils.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define USERNAME_SIZE 20 /* Max size for username */
#define TITLE_SIZE 20 /* Max size for title */
/* Buffer size for the response to to the client
NOTE: response size may be bigger depending on the max number of events being
retrieved from a user's specific date */
#define RESPONSE_SIZE 128

LIST* users;
pthread_mutex_t usersLock;

/* Initializes the calendar database */
void initCalendar() {
    users = ListCreate();
    pthread_mutex_init(&usersLock, NULL);
}

/*
    Returns the client's request type based on the message provided
    UNKNOWN if somehow the request doesnt match <- Shouldn't happen
*/
RequestType processRequestType(const char *message) {
    const char *request;
    
    const char *offset = strchr(message, ' ');
    if (!offset) return UNKNOWN;
    
    request = offset + 1;

    if (strncmp(request, "ADD", 3) == 0) return ADD;
    if (strncmp(request, "REMOVE", 6) == 0) return REMOVE;
    if (strncmp(request, "UPDATE", 6) == 0) return UPDATE;
    if (strncmp(request, "GETEVENT", 8) == 0) return GETEVENT;
    if (strncmp(request, "GETALL", 6) == 0) return GETALL;
    if (strncmp(request, "GET", 3) == 0) return GET;

    return UNKNOWN;
}

/*
    Gets the user from the client's request and returns the user that matches
    Creates a new user if this user isnt already registered and returns it
*/
User* processUser(const char* request) {
    char username[USERNAME_SIZE] = {0};
    User* user;

    sscanf(request, "%19s", username);

    /* Look for user in users list */
    pthread_mutex_lock(&usersLock);
    for (user = (User*)ListFirst(users); 
         user != NULL; user = (User*)ListNext(users)) 
    {
        if (strcmp(user->username, username) == 0) {
            pthread_mutex_unlock(&usersLock);
            printf("Utils: user found\n"); /* TEST */
            return user;
        }
    }

    /* Create new user if it isnt found */
    user = createUser(username);
    if (!user) {
        pthread_mutex_unlock(&usersLock);
        return NULL;
    }
    ListAppend(users, user);
    
    pthread_mutex_unlock(&usersLock);
    printf("Utils: new user created\n"); /* TEST */
    return user;
}

/*
    Gets the attributes from the client message and assigns them to the provided
    char pointers, sets the default of each attribute to an empty string if
    attribute isnt provided in the message
*/
void processAttributes(const char* message, 
                       char* date, char* start, char* end, char* title) {

    const char* offset = strchr(message, ' ');
    if (!offset) return;
    
    date[0] = '\0'; start[0] = '\0'; end[0] = '\0'; title[0] = '\0';

    offset = strchr(offset + 1, ' ');
    if (!offset) return;


    sscanf(offset + 1, "%10s %4s %4s %19s", date, start, end, title);
}

/* Check if request type is a read so the child can process for server_proc */
int isRead(const char* message){
    RequestType request = processRequestType(message);
    
    if (request == UNKNOWN) return -1;
    if (request == ADD ||
        request == REMOVE ||
        request == UPDATE) 
    {
        return 0;
    }
    return 1;
}

/* Check if request type is GETALL for the multi-proc and multi-thread server */
int isGetall(const char* message){
    RequestType request = processRequestType(message);
    
    if (request == UNKNOWN) return -1;
    if (request == GETALL) 
    {
        return 1;
    }
    return 0;
}

User* lockUser(const char* message) {
    char username[USERNAME_SIZE] = {0};
    User* user;

    sscanf(message, "%19s", username);

    /* Look for user in users list */
    pthread_mutex_lock(&usersLock);
    for (user = (User*)ListFirst(users); 
         user != NULL; user = (User*)ListNext(users)) 
    {
        if (strcmp(user->username, username) == 0) {
            pthread_mutex_unlock(&usersLock);
            printf("Utils: user found\n"); /* TEST */
            pthread_mutex_lock(&user->mutex);
            return user;
        }
    }
    pthread_mutex_unlock(&usersLock);
    return NULL;
}

void unlockUser(User* user) {
    pthread_mutex_unlock(&user->mutex);
}

/*
    Takes the message sent by the client and processes the server's response
*/
char* processRequest(const char* request) {
    char dateOrIndex[11] = {0}, start[5] = {0}, end[5] = {0}, title[20] = {0};

    Event* event;
    User* user;
    
    char *response = (char*)malloc(RESPONSE_SIZE);

    /* Initialize the calendar application */
    if (!users) initCalendar();

    /* Get the user and attributes from the client message */
    processAttributes(request, dateOrIndex, start, end, title);
    user = processUser(request);
    printf("Utils: attributes read from client request\n"); /* TEST */

    /* 
        switch case for each request type
        For each case, calls the appropriate calendar method 
        with its required parameters
    */
    switch(processRequestType(request)){
    case ADD:
    printf("Utils: calling ADD from calendar.a\n"); /* TEST */
        /* Adds an event */
        if (addEvent(user, dateOrIndex, start, end, title) == 0) {
            snprintf(response, RESPONSE_SIZE, "Event added successfully");
        } else snprintf(response, RESPONSE_SIZE, "OVERLAP: Adding failed");
        break;

    case REMOVE:
    printf("Utils: calling REMOVE from calendar.a\n"); /* TEST */
        /* Removes an event */
        if (removeEvent(user, dateOrIndex, start) == 0) {
            snprintf(response, RESPONSE_SIZE, "Event removed successfully");
        } else  snprintf(response, RESPONSE_SIZE, "Removing event failed");
        break;

    case UPDATE:
    printf("Utils: calling UPDATE from calendar.a\n"); /* TEST */
        /* Updates an event */
        if (updateEvent(user, dateOrIndex, start, end, title) == 0) {
            snprintf(response, RESPONSE_SIZE, "Event updated successfully");
        } 
        else {
            snprintf(response, RESPONSE_SIZE, "Updating event failed.\n"\
                "Overlap detected, or event not found");
        }
        break;

    case GET:
    printf("Utils: calling GET from calendar.a\n"); /* TEST */
        /* Get all events for date if start time not provided */
        if (start[0] == '\0') {
            int i, eventsForDate = getEventCountForDate(user, dateOrIndex);
            Event** events = getEventsForDate(user, dateOrIndex);

            /* Structure a response containingthe event titles for date if any
            Titles seperated by ',' */
            if (events) { 
                char *ptr;
                char *tempResponse = realloc(response,
                    (TITLE_SIZE * eventsForDate) + (2 * (eventsForDate)));
                
                if (tempResponse == NULL) {
                    free(response);
                    free(events);
                    response = NULL;
                    break;
                } else {
                    response = tempResponse;
                    ptr = response;
                }
                for (i = 0; i < eventsForDate; i++) {
                    strcpy(ptr, events[i]->title);
                    ptr += strlen(events[i]->title);

                    if (i < eventsForDate - 1) {
                        strcpy(ptr, ", ");
                        ptr += 2;
                    }
                }
                *ptr = '\0';

                free(events);     
            } else {
                snprintf(response, RESPONSE_SIZE, 
                    "No events on %s", dateOrIndex);
            }
        } else { /* Get the event specified if start time is provided */
            event = getEvent(user, dateOrIndex, start);
            if (event) {
                snprintf(response, RESPONSE_SIZE, event->title);
            } else snprintf(response, RESPONSE_SIZE, "Event does not exist");
        } 
        break;

    case GETALL:
    printf("Utils: calling GETALL from calendar.a\n"); /* TEST */
        /* Gets the event count to send to the client */
        snprintf(response, RESPONSE_SIZE, "COUNT:%d", getEventCount(user));
        break;

    case GETEVENT:
    printf("Utils: calling GETEVENT from calendar.a\n"); /* TEST */
        /* Gets the event at specified index */
        event = getEventByIndex(user, atoi(dateOrIndex));
        if (event) {
            snprintf(response, RESPONSE_SIZE, "%s", event->title);
        } else {
            snprintf(response, RESPONSE_SIZE, 
                "Getting event %s failed", dateOrIndex);
        }
        break;

    case UNKNOWN:
        /* Inform reading request failed if the message was corrupted somehow */
        snprintf(response, RESPONSE_SIZE, "Reading request type failed");
        break;
    }

    return response;
}

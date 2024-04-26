/*
    David Simonov
    das892
    11301273
*/


#include <list.h>
#include <pthread.h>

/* How the Date is structured */
typedef struct {
    int day;
    int month;
    int year;
} Date;

/* How an Event is structured */
typedef struct {
    char title[20];
    Date date;
    int start;
    int end;
} Event;

/* How a User is structured */
typedef struct {
    char username[20];
    LIST* events;
    pthread_mutex_t mutex;
} User;

int getEventCount(User *user);
int getEventCountForDate(User *user, char* date);
int addEvent(User *user, char* date, char* start, char* end, char* title);
int removeEvent(User *user, char* date, char* start);
int updateEvent(User *user, char* date, char* start, char* end, char* title);
Event* getEvent(User *user, char* date, char* start);
Event* getEventByIndex(User *user, int i);
Event** getEventsForDate(User *user, char* date);
User* createUser(char* userName);
void deleteUser(User* user);

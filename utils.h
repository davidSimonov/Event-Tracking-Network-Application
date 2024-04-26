/*
    David Simonov
    das892
    11301273
*/

#include <calendar.h>

/* enum for all the different client request types */
typedef enum {
    ADD,
    REMOVE,
    UPDATE,
    GET,
    GETALL,
    GETEVENT,
    UNKNOWN
} RequestType;

char* processRequest(const char*);
User* lockUser(const char*);
void unlockUser(User*);
int isRead(const char*);
int isGetall(const char*);

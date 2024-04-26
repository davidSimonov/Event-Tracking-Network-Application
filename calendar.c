/*
    David Simonov
    das892
    11301273
*/

#include <calendar.h>
#include <string.h>

/* enum for all the different relationships events can have between eachother */
typedef enum {
    AFTER,
    BEFORE,
    OVERLAP
} State;

/*
    Takes a username
    Returns a new user created
*/
User* createUser(char* userName) {
    User* newUser;

    if (userName == NULL) {
        return NULL;
    }
    printf("Calendar: creating user...\n"); /* TEST */

    newUser = (User*)malloc(sizeof(User));

    strncpy(newUser->username, userName, sizeof(newUser->username) - 1);
    newUser->username[sizeof(newUser->username) - 1] = '\0';

    newUser->events = ListCreate();
    if (newUser->events == NULL) {
        free(newUser);
        return NULL;
    }
    pthread_mutex_init(&newUser->mutex, NULL);

    return newUser;
}

/*
    Takes a User
    Deletes the user
*/
void deleteUser(User* user) {
    Event* currEvent;
    
    if (user == NULL) {
        return;
    }

    pthread_mutex_lock(&user->mutex);

    currEvent = (Event*) ListFirst(user->events);
    while ((currEvent = (Event*)ListTrim(user->events)) != NULL) {
        free(currEvent);
    }

    ListFree(user->events, NULL);
    pthread_mutex_destroy(&user->mutex);
    free(user);
}

/*
    Takes two events
    Returns the date relationship between them
*/
State dateCmp(Event *event1, Event *event2) {
    int day1, month1, year1, day2, month2, year2;
    day1 = event1->date.day;
    month1 = event1->date.month;
    year1 = event1->date.year;
    day2 = event2->date.day;
    month2 = event2->date.month;
    year2 = event2->date.year;

    if (year1 > year2) {
        return AFTER;
    } else if (year1 < year2) {
        return BEFORE;
    } else {
        if (month1 > month2) {
            return AFTER;
        } else if (month1 < month2) {
            return BEFORE;
        } else {
            if (day1 > day2) {
                return AFTER;
            } else if (day1 < day2) {
                return BEFORE;
            } else {
                return OVERLAP;
            }
        }
    }
}

/*
    Takes two events
    Returns the time relationship between them
*/
State timeCmp(Event *event1, Event *event2) {
    int start1, end1, start2, end2;
    start1 = event1->start;
    end1 = event1->end;
    start2 = event2->start;
    end2 = event2->end;
    

    if (end1 < start2) {
        return BEFORE;
    } else if (start1 > end2) {
        return AFTER;
    } else {
        return OVERLAP;
    }
}

/*
    Takes two events
    Returns the relationship between them
*/
State eventCmp(Event *event1, Event *event2) {

    State dates = dateCmp(event1, event2);

    if (dates == OVERLAP) {
        return timeCmp(event1, event2);
    } return dates;
}

/*
    Takes the date as string and 3 int pointers for the day, month and year
    Populates the in pointers with the day, month and year provided in date
*/
void dateToInts(char* date, int* day, int* month, int* year) {
    char dd[3], mm[3], yyyy[5];

    strncpy(dd, date, 2);
    dd[2] = '\0';
    strncpy(mm, date + 3, 2);
    mm[2] = '\0';
    strncpy(yyyy, date + 6, 4);
    yyyy[4] = '\0';

    *day = atoi(dd);
    *month = atoi(mm);
    *year = atoi(yyyy);
}

/*
    Takes a User
    Returns the number of events registered to that user
*/
int getEventCount(User *user) {
    int count;

    printf("Calendar: getting total event count...\n"); /* TEST */
    
    count = ListCount(user->events);

    return count;
}

/*
    Takes a User and a date
    Returns the number of events registered to that user for that date
*/
int getEventCountForDate(User *user, char* date) {
    int day, month, year, count = 0;
    
    LIST* eventList = user->events;
    Event* currEvent;
    Event tempEvent;

    dateToInts(date, &day, &month, &year);
    tempEvent.date.day = day;
    tempEvent.date.month = month;
    tempEvent.date.year = year;

    pthread_mutex_lock(&user->mutex);

    currEvent = (Event*) ListFirst(eventList);
    while (currEvent != NULL) {
        State events = dateCmp(currEvent, &tempEvent);
        if (events == AFTER) break;
        else if (events == OVERLAP) count++;
        
        currEvent = (Event*) ListNext(eventList);
    }

    pthread_mutex_unlock(&user->mutex);
    return count;
}

/*
    Takes a User and the arguments for adding an event
    Creates a new event and adds it to the event list for the user
    Returns 0 on success, -1 if the user has an event that overlaps with the new
*/
int addEvent(User *user, char* date, char* start, char* end, char* title) {
    int day, month, year;

    LIST* eventList;
    Event* newEvent = (Event *)malloc(sizeof(Event));
    Event* currEvent;
    State events;

    printf("Calendar: adding new event...\n"); /* TEST */
    /* Extract the day, month, and year from the date string */
    dateToInts(date, &day, &month, &year);
    newEvent->date.day = day;
    newEvent->date.month = month;
    newEvent->date.year = year;

    /* Populate the new event */
    strncpy(newEvent->title, title, sizeof(newEvent->title) - 1);
    newEvent->title[sizeof(newEvent->title) - 1] = '\0';
    newEvent->start = atoi(start);
    newEvent->end = atoi(end);

    pthread_mutex_lock(&user->mutex);

    eventList = user->events;
    currEvent = (Event*) ListFirst(eventList);
    
    /* Loop through the event list to find where to place the event */
    if (currEvent == NULL || eventCmp(newEvent, currEvent) == BEFORE) {
        ListInsert(eventList, newEvent);
        pthread_mutex_unlock(&user->mutex);
        return 0;
    }

    do {
        currEvent = (Event*)ListCurr(eventList);
        events = eventCmp(newEvent, currEvent);
        
        if (events == OVERLAP) { /* Return -1 if event overlaps */
            free(newEvent);
            pthread_mutex_unlock(&user->mutex);
            return -1;
        /* Continue looping if events are still before the new event */
        } else if (events == BEFORE) {
            ListInsert(eventList, newEvent);
            pthread_mutex_unlock(&user->mutex);
            return 0;
        }
    } while (ListNext(eventList) != NULL);

    /* Add the event once the right position has been found in the list */
    ListAdd(eventList, newEvent);
    pthread_mutex_unlock(&user->mutex);
    return 0;
}

/*
    Removes an event from a user's event list based on the date and start time.
    Returns 0 on successful removal, -1 if no matching event was found.
*/
int removeEvent(User *user, char* date, char* start) {
    /* Variable declarations for day, month, year, and startTime */
    int day, month, year, startTime;

    /* Access the user's list of events */
    LIST* eventList = user->events;
    Event* currEvent;

    printf("Calendar: removing event...\n"); /* TEST */
    /* Convert date string to integers */
    dateToInts(date, &day, &month, &year);
    startTime = atoi(start);

    /* Lock the user's mutex to ensure thread-safe access */
    pthread_mutex_lock(&user->mutex);

    /* Check if the event list is empty */
    if (ListFirst(eventList) == NULL) {
        pthread_mutex_unlock(&user->mutex);
        return -1;
    }

    /* Loop through the event list to find the matching event */
    do {
        currEvent = (Event*) ListCurr(eventList);
        if (currEvent->date.day == day && 
            currEvent->date.month == month && 
            currEvent->date.year == year && 
            currEvent->start == startTime) 
        {
            ListRemove(eventList); /* Remove the event */
            free(currEvent); /* Free the removed event's memory */
            pthread_mutex_unlock(&user->mutex); /* Unlock the mutex */
            return 0; /* Indicate success */
        }
    } while (ListNext(eventList) != NULL);

    pthread_mutex_unlock(&user->mutex); /* Unlock the mutex */
    return -1; /* Indicate failure if no matching event was found */
}

/*
    Updates an event in a user's list at date and start time provided.
    The event is updated with the new end time and title.
    Returns 0 on successful update, 
    -1 if the update is not possible due to overlap with another event 
    or if no matching event was found.
*/
int updateEvent(User *user, char* date, char* start, char* end, char* title) {
    int day, month, year, startTime, newEndTime;

    LIST* eventList = user->events;
    Event* currEvent;
    Event* nextEvent;

    printf("Calendar: updating event...\n"); /* TEST */
    /* Convert date and time strings to integers */
    dateToInts(date, &day, &month, &year);
    startTime = atoi(start);
    newEndTime = atoi(end);

    pthread_mutex_lock(&user->mutex);

    /* Loop through the event list to find the matching event */
    currEvent = (Event*) ListFirst(eventList);
    while (currEvent != NULL) {
        if (currEvent->date.day == day && 
            currEvent->date.month == month && 
            currEvent->date.year == year && 
            currEvent->start == startTime) 
        {
            nextEvent = (Event*) ListNext(eventList);
            /* Check for overlap with the next event */
            if (nextEvent && nextEvent->start < newEndTime) {
                pthread_mutex_unlock(&user->mutex);
                return -1; /* Indicate failure due to overlap */
            }

            /* Update the event with new end time and title */
            currEvent->end = newEndTime;
            strncpy(currEvent->title, title, sizeof(currEvent->title) - 1);
            currEvent->title[sizeof(currEvent->title) - 1] = '\0';
            pthread_mutex_unlock(&user->mutex);
            return 0; /* Indicate success */
        }
        currEvent = (Event*) ListNext(eventList); /* Move to the next event */
    }

    pthread_mutex_unlock(&user->mutex);
    return -1; /* Indicate failure if no matching event was found */
}


/*
    Retrieves a specific event for a user based on the date and start time.
    Returns a pointer to the event if found, NULL otherwise.
    Assumes user is loked
*/
Event* getEvent(User *user, char* date, char* start) {
    int day, month, year, startTime;
    
    LIST* eventList = user->events;
    Event* currEvent;

    printf("Calendar: getting event...\n"); /* TEST */
    /* Convert date string to integer components */
    dateToInts(date, &day, &month, &year);
    startTime = atoi(start);

    /* Check if the event list is empty */
    if (ListFirst(eventList) == NULL) {
        pthread_mutex_unlock(&user->mutex);
        return NULL;
    }

    /* Iterate through the event list to find the matching event */
    do {
        currEvent = (Event*) ListCurr(eventList);
        if (currEvent->date.day == day && 
            currEvent->date.month == month && 
            currEvent->date.year == year && 
            currEvent->start == startTime) 
        {
            pthread_mutex_unlock(&user->mutex);
            return currEvent;
        }
    } while (ListNext(eventList) != NULL);

    return NULL;
}

/*
    Retrieves an event from a user's list by index.
    Returns a pointer to the event if found, NULL if index is out of bounds.
*/
Event* getEventByIndex(User *user, int i) {
    int count = 1;

    LIST* eventList = user->events;
    Event* currEvent;

    printf("Calendar: getting event at index %d...\n", i); /* TEST */

    /* Loop to find the event at the specified index */
    currEvent = (Event*) ListFirst(eventList);
    while (currEvent != NULL && count < i) {
        currEvent = (Event*) ListNext(eventList);
        count++;
    }

    return currEvent;
}

/*
    Retrieves all events for a user on a specific date.
    Returns an array of pointers to events, NULL for no events found.
*/
Event** getEventsForDate(User *user, char* date){
    int day, month, year, index = 0;

    LIST* eventList = user->events;
    Event** eventsForDate;
    Event* currEvent;
    Event tempEvent;
    
    /* Calculate the total number of events for the date */
    int eventCount = getEventCountForDate(user, date);
    if (eventCount == 0) {
        return NULL;
    }
    printf("Calendar: getting all events for date %s\n", date); /* TEST */

    /* Allocate memory for the array of event pointers */
    eventsForDate = (Event**)malloc((eventCount + 1) * sizeof(Event*));

    /* Convert date string to integer components */
    dateToInts(date, &day, &month, &year);
    tempEvent.date.day = day;
    tempEvent.date.month = month;
    tempEvent.date.year = year;

    pthread_mutex_lock(&user->mutex);

    /* Find events that match the specified date */
    currEvent = (Event*) ListFirst(eventList);
    while (currEvent != NULL && index < eventCount) {
        State events = dateCmp(currEvent, &tempEvent);
        /* Exit loop if current event is after the target date */
        if (events == AFTER) break;
        /* Add event to the array if it matches the date */
        else if (events == OVERLAP) eventsForDate[index++] = currEvent;

        currEvent = (Event*) ListNext(eventList);
    } 
    eventsForDate[eventCount] = NULL;

    pthread_mutex_unlock(&user->mutex);
    return eventsForDate;
}
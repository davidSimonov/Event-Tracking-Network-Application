/*Header file*/
/* Tommy Ojo, odo128, 11292921, Lab4 */
/* David Simonov, das892, 11201273 */

/*WE ARE DOING THE BONUS, LOOK AT SECTION 6 IN THE DESIGN DOC FOR AN 
EXPLANATION*/



#include <stdlib.h>
#include <stdio.h>


typedef struct node {
    void* item; /* Item being held by the node*/
    void* next; /* next node*/
    void* prev; /* previous node*/
} NODE;

typedef struct list{
    /* Start of the list*/
    NODE* start;
    /* End of the list */
    NODE* end;
    /* Cursor value (current item)*/
    NODE* currentNode;
    /* Number of nodes*/
    int num_of_nodes;
    /*1 if list has been called in ListFree, 0 otherwise*/
    int init;
    /*Memory address that represents where it would be in a stationary memory 
    block if the list expands/grows without moving */
    void* og_addr;
} LIST;

typedef int (*Comparator)(void*, void*);

typedef void (*Itemfree)(void*);

/* function definitions */

LIST *ListCreate();

int ListCount(LIST* list);

void *ListFirst(LIST* list);

void *ListLast(LIST* list);

void *ListNext(LIST* list);

void *ListPrev(LIST* list);

void *ListCurr(LIST* list);

int ListAdd(LIST* list,void* item);

int ListInsert(LIST* list, void * item);

int ListAppend(LIST* list, void* item);

int ListPrepend( LIST* list, void* item);

void *ListRemove(LIST* list);

void ListConcat( LIST* list1, LIST* list2);

void ListFree(LIST* list, Itemfree itemFree);

void *ListTrim(LIST* list);

void *ListBehead(LIST* list);

void *ListSearch(LIST* list, Comparator comparator,void* comparisonArg);

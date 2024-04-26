/* Tommy Ojo, odo128, 11292921, A1-P3
David Simonov, das892, 11201273*/

/*WE ARE DOING THE BONUS, LOOK AT SECTION 6 IN THE DESIGN DOC FOR AN 
EXPLANATION*/


#include <stdlib.h>
#include <stdio.h>
#include <list.h>

/* Keeps track of the amount of Lists and Nodes*/
int NODE_COUNT = 0;
int LIST_COUNT = 0;

/* Keeping track of moved nodes and lists*/
int REMOVED_NODE_COUNT = 0;
int REMOVED_LIST_COUNT = 0;


/*Minimum amount to allocate for structures*/
int MIN_LISTS = 50;
int MIN_NODES = 500;

/* Multiplier for if LIST/NODE runs out of memory*/
int LIST_MULTIPLIER = 1;
int NODE_MULTIPLIER = 1;

int make_malloc = 1; /*make malloc for the first ListCreate*/

int pointer_adjust_list = 0;
int pointer_adjust_node = 0;


/*Main memory blocks for list and node*/
LIST* mainlist_mem;
NODE* mainnode_mem;
LIST* new_mainlist_mem = NULL;
NODE* new_mainnode_mem = NULL;

/*Main memory blocks for removed list and node*/
LIST* removedlist_mem[5000];
NODE* removednode_mem[5000];

/* Array for the addresses of the lists
I am using one list for the key/values to avoid multiple mallocs
The keys are the even indexes and the corresponding value to a key is the index 
to the right 
i.e 2n = keys, 2n+1 = values */
LIST* real_addr_list[10000];

int list_shrunk = 0;

/* makes a new, empty list, and returns its reference on success. 
Returns a NULL pointer on failure. */
LIST *ListCreate(){
    LIST* list;
    LIST* list_mem_max;
    int i; /*used in for loop*/
    LIST* real_addr;
    LIST* list_og_addr; /*the same value as list unless list pool has shrunk*/


    if (make_malloc ==1){
         mainlist_mem = (LIST*) malloc(sizeof(LIST)*MIN_LISTS);
         mainnode_mem = (NODE*) malloc(sizeof(NODE)*MIN_NODES);
         new_mainlist_mem = mainlist_mem;
         new_mainnode_mem = mainnode_mem;
         make_malloc = 0;
    }


    /*Max list memory*/
    list_mem_max = mainlist_mem + MIN_LISTS*LIST_MULTIPLIER;

     /*If adding a new list would be out of bounds of the memory:
     Expand the memory */  
    if ((REMOVED_LIST_COUNT == 0) && (mainlist_mem+LIST_COUNT >= list_mem_max)){
        printf("Lists are out of memory, reallocating more \n"); 
        /*out of memory to allocate, doubling MIN_LISTS*/
        LIST_MULTIPLIER = LIST_MULTIPLIER*2;
        /*reallocing double the space*/
        new_mainlist_mem = realloc(new_mainlist_mem, 
        sizeof(LIST)*MIN_LISTS*LIST_MULTIPLIER);
        /* Storing the difference between oldmainlist mem and new mainlistmem
        Need to convert to long int to make the pointer arithemethic work*/
        pointer_adjust_list = ((long int)new_mainlist_mem) - 
        ((long int)mainlist_mem);

        /*Since we are now using the real adresses list, we have to go back and 
        fix all the list addresses*/
        for(i=0; i < LIST_COUNT; i++){
            real_addr_list[(i*2)+1] = new_mainlist_mem+i;
        }

    }



    /* Getting the memory address for the new list*/
    if (REMOVED_LIST_COUNT == 0){
        list = mainlist_mem + LIST_COUNT;
    }
    else{
        REMOVED_LIST_COUNT--;
        list = removedlist_mem[REMOVED_LIST_COUNT];
    }
    LIST_COUNT++;

    /* Real address for the list that was created*/
    real_addr=  (LIST*) (((long int)list) + pointer_adjust_list);

    real_addr->num_of_nodes = 0;
    real_addr->init = 1;
    real_addr->currentNode = NULL;
    real_addr->start = NULL;
    real_addr->end = NULL;

    /* If list has shrunk, the pointers are all messed up but we still have to 
    give each list a unique pointer. If list to be given at this position 
    already exists in the list, double the number and check again continously*/
    list_og_addr = list;
    if (list_shrunk){
        for(i=0;i < LIST_COUNT; i++){
            if(list_og_addr == real_addr_list[i*2]){
                list_og_addr = (LIST*) (((long int)list_og_addr)*2);
                i = 0;
            }
        }
    }

    /*Inserting the address being given to the variable (key) into the list at 
    an even index
    (list-mainlist_mem) is used instead of LIST_COUNT to ensure that the order 
    of addresses in the list matches the order they are in for the list pools*/
    real_addr_list[(list-mainlist_mem)*2] = list_og_addr;

    /*Inserting the values, the real addresses*/
    real_addr_list[((list-mainlist_mem)*2)+1] = real_addr;


    real_addr->og_addr = list_og_addr;

    return list_og_addr;
}

/* adds the new item to list directly after the current item, and 
makes the new item the current item. If the current pointer 
is at the end of the list, the item is added at the end. 
Returns 0 on success, -1 on failure. */
int ListAdd(LIST* list,void* item){
    NODE* next; 
    NODE* curr; /*Real address of current node*/
    NODE* newNode; 
    NODE* real_addr; /*Real address for newly added node*/
    LIST* mylist; /* Real address for the list*/
    int k; /*used in for loop*/

    /*If List has shrunk, switch to using the real addresses list instead of 
    pointer arithmetic*/
    if (list_shrunk){
        for(k = 0; k < LIST_COUNT; k++){
            if(real_addr_list[k*2]== list){
                mylist = real_addr_list[(2*k)+1];
                k = LIST_COUNT; /* Breaks the loop*/
            }
        }        
    }
    else{
        /*Adjusting the pointer being referenced base depending on if the 
        pointer has been moved. 
        - If it has not been moved then pointer_adjust_list = 0, and this 
        changes nothing. 
        - If it has been moved, then pointer_adjust_list = the difference 
        between the new memory and the original memory and mylist will 
        reference newmemory 
        - Trust me, you HAVE to do the typecasts like this. If you do not it is 
        your own funeral*/
        mylist = (LIST*) (((long int)list) + pointer_adjust_list);
        
    }

    /* The actual values of the currentNode are in this address */
    curr = (NODE*) (((long int)mylist->currentNode) + pointer_adjust_node); 
    

    
   
    /*If out of space for node memory*/
    if ((REMOVED_NODE_COUNT == 0) && (mainnode_mem + NODE_COUNT >= mainnode_mem 
    + MIN_NODES*NODE_MULTIPLIER)){
        printf("Nodes are out of memory, reallocating more \n"); 
        /*out of memory to allocate, doubling MIN_NODES*/
        NODE_MULTIPLIER = NODE_MULTIPLIER*2;
        new_mainnode_mem = realloc(new_mainnode_mem, 
        sizeof(NODE)*MIN_NODES*NODE_MULTIPLIER);
        /* Storing the difference between mainnode mem and new mainnodemem
        Need to convert to long int to make the pointer arithemethic work*/
        pointer_adjust_node = ((long int)new_mainnode_mem) - 
        ((long int)mainnode_mem);
    }

    /* Getting the memory address for the new node*/
    /*If there are no removed nodes that can be used*/
    if (REMOVED_NODE_COUNT == 0){
        newNode = mainnode_mem + NODE_COUNT;
    }
    else{
        REMOVED_NODE_COUNT--;
        newNode = removednode_mem[REMOVED_NODE_COUNT];
    }

    real_addr = (NODE*) (((long int)newNode) + pointer_adjust_node);



    if (mylist->num_of_nodes == 0){
        real_addr->item = item; 
        /* give the currentNode the addresses that coresponds to the original 
        mainnode_mem*/
        mylist->currentNode = newNode;
        mylist->start  = newNode;
        mylist->end = newNode;
    }
    /*If there is only one node or we are at the end of the list*/
    else if (mylist->currentNode == mylist->end){  

        /* making a new node and adding the item into the real address*/
        real_addr->item = item; 

        /*adding links for new node*/
        real_addr->prev = curr;

        /*fixing links for old nodes*/
        curr->next = real_addr;

        /* setting current position to the newly added node*/
        mylist->currentNode = newNode; 

        mylist->end = mylist->currentNode;
    
    }
    else{
        real_addr->item = item; 

        next = curr->next;


        /*adding links for new node*/
        real_addr->next = next;
        real_addr->prev = curr;

        /*fixing links for old nodes*/
        curr->next = real_addr;
        next->prev = real_addr;

        /* setting current position to the newly added node*/
        mylist->currentNode = newNode;
    }

    NODE_COUNT++;
    

    /* If nothing went wrong...*/
    if (real_addr->item == item){ 
        mylist->num_of_nodes++; /*increment number of nodes*/
        return 0;
    }
    else{
        return -1;
    }

}


/*adds item to list directly before the current item, and makes the 
new item the current one. If the current pointer is at the start of 
the list, the item is added at the start.  
Returns 0 on success, -1 on failure. */
int ListInsert(LIST* list, void * item){
    NODE* prev;
    NODE* newNode;
    NODE* curr;
    NODE* real_addr;
    LIST* mylist;
    int k;

    if (list_shrunk){
        for(k = 0; k < LIST_COUNT; k++){
            if(real_addr_list[k*2]== list){
                mylist = real_addr_list[(2*k)+1];
                k = LIST_COUNT; /* Breaks the loop*/
            }
        }
    }
    else{
        mylist = (LIST*) (((long int)list) + pointer_adjust_list);
    }

    curr = (NODE*) (((long int)mylist->currentNode) + pointer_adjust_node);




    if ((REMOVED_NODE_COUNT == 0) && (mainnode_mem + NODE_COUNT >= mainnode_mem 
    + MIN_NODES*NODE_MULTIPLIER)){
        printf("Nodes are out of memory, reallocating more \n"); 
        /*out of memory to allocate, doubling MIN_NODES*/
        NODE_MULTIPLIER = NODE_MULTIPLIER*2;
        new_mainnode_mem = realloc(new_mainnode_mem, 
        sizeof(NODE)*MIN_NODES*NODE_MULTIPLIER);
        /* Storing the difference between mainnode mem and new mainnodemem
        Need to convert to long int to make the pointer arithemethic work*/
        pointer_adjust_node = ((long int)new_mainnode_mem) - 
        ((long int)mainnode_mem);
    }

    /* Getting the memory address for the new node*/
    if (REMOVED_NODE_COUNT == 0){
    
        newNode = mainnode_mem + NODE_COUNT;
    }
    else{
        REMOVED_NODE_COUNT--;
        newNode = removednode_mem[REMOVED_NODE_COUNT];
    }
    NODE_COUNT++;

    /*the actual address of the node*/
    real_addr = (NODE*) (((long int)newNode) + pointer_adjust_node);



    if (mylist->num_of_nodes == 0){
        real_addr->item = item;
        mylist->currentNode = newNode;
        mylist->start = newNode;
        mylist->end = newNode;
    }
    /*If there is only one node or we are at the start of the list*/
    else if (mylist->currentNode == mylist->start){ 
        /* making a new node and inserting 
        the item into the newnode*/
        real_addr->item = item; 

        /*adding links for new node*/
        real_addr->next = curr;

        /*fixing links for old nodes*/
        curr->prev = real_addr;

        /* setting current position to t
        he newly inserted node*/
        mylist->currentNode = newNode; 
        mylist->start = mylist->currentNode;

    } 
    else{
        /* making a new node and 
        inserting the item into the newnode*/
        real_addr->item = item; 
        prev = curr->prev;

        /*adding links for new node*/
        real_addr->prev = prev;
        real_addr->next = curr;

        /*fixing links for old nodes*/
        curr->prev = real_addr;
        prev->next = real_addr;

        /* setting current position 
        to the newly inserted node*/
        mylist->currentNode = newNode; 
    }
    
    /* If nothing went wrong...*/
    if (real_addr->item == item){ 
        mylist->num_of_nodes++; /*increment number of nodes*/
        return 0;
    }
    else{
        return -1;
    }

}

/*  adds item to the end of list, and makes the new item the current
 one. Returns 0 on success, -1 on failure. */
int ListAppend(LIST* list, void* item){
    NODE* newNode;
    NODE* end;
    NODE* real_addr;
    LIST* mylist;
    int k;

    if (list_shrunk){
        for(k = 0; k < LIST_COUNT; k++){
            if(real_addr_list[k*2]== list){
                mylist = real_addr_list[(2*k)+1];
                k = LIST_COUNT; /* Breaks the loop*/
            }
        }
    }
    else{
        mylist = (LIST*) (((long int)list) + pointer_adjust_list);
    }

    end = (NODE*) (((long int)mylist->end) + pointer_adjust_node);
   
    if ((REMOVED_NODE_COUNT == 0) && (mainnode_mem + NODE_COUNT >= mainnode_mem 
    + MIN_NODES*NODE_MULTIPLIER)){
        printf("Nodes are out of memory, reallocating more \n"); 
        /*out of memory to allocate, doubling MIN_NODES*/
        NODE_MULTIPLIER = NODE_MULTIPLIER*2;
        new_mainnode_mem = realloc(new_mainnode_mem, 
        sizeof(NODE)*MIN_NODES*NODE_MULTIPLIER);
        /* Storing the difference between mainnode mem and new mainnodemem
        Need to convert to long int to make the pointer arithemethic work*/
        pointer_adjust_node = ((long int)new_mainnode_mem) - 
        ((long int)mainnode_mem);
    }

    /* Getting the memory address for the new node*/
    if (REMOVED_NODE_COUNT == 0){
    
        newNode = mainnode_mem + NODE_COUNT;
    }
    else{
        REMOVED_NODE_COUNT--;
        newNode = removednode_mem[REMOVED_NODE_COUNT];
    }
    NODE_COUNT++;

    /*the actual address of the node*/
    real_addr = (NODE*) (((long int)newNode) + pointer_adjust_node);

    
    if (mylist->num_of_nodes == 0){
        real_addr->item = item;
        mylist->currentNode = newNode;
        mylist->start = newNode;
        mylist->end = newNode;
    }
    else{
        /* making a new node and adding 
        the item into the newnode*/
        real_addr->item = item; 
        
        /*adding links for new node*/
        real_addr->prev = end;

        /*fixing links for old nodes*/
        end->next = real_addr;


        /*updating old end to new end and 
        setting it to the current position*/
        mylist->end = newNode;
        mylist->currentNode = mylist->end;
    }


    /* If nothing went wrong...*/
    if (real_addr->item == item){ 
        mylist->num_of_nodes++; /*increment number of nodes*/
        return 0;
    }
    else{
        return -1;
    }

}


/* adds item to the front of list, and makes the new item the current 
one. Returns 0 on success, -1 on failure.*/
int ListPrepend( LIST* list, void* item){
    NODE* newNode;
    NODE* start;
    NODE* real_addr;
    LIST* mylist;
    int k;
    

    if (list_shrunk){
        for(k = 0; k < LIST_COUNT; k++){
            if(real_addr_list[k*2]== list){
                mylist = real_addr_list[(2*k)+1];
                k = LIST_COUNT; /* Breaks the loop*/
            }
        }
    }
    else{
        mylist = (LIST*) (((long int)list) + pointer_adjust_list); 
    }

    start = (NODE*) (((long int)mylist->start) + pointer_adjust_node);

    if ((REMOVED_NODE_COUNT == 0) && (mainnode_mem + NODE_COUNT >= mainnode_mem 
    + MIN_NODES*NODE_MULTIPLIER)){
        printf("Nodes are out of memory, reallocating more \n"); 
        /*out of memory to allocate, doubling MIN_NODES*/
        NODE_MULTIPLIER = NODE_MULTIPLIER*2;
        new_mainnode_mem = realloc(new_mainnode_mem, 
        sizeof(NODE)*MIN_NODES*NODE_MULTIPLIER);
        /* Storing the difference between mainnode mem and new mainnodemem
        Need to convert to long int to make the pointer arithemethic work*/
        pointer_adjust_node = ((long int)new_mainnode_mem) - 
        ((long int)mainnode_mem);
    }

   /* Getting the memory address for the new node*/
    if (REMOVED_NODE_COUNT == 0){
    
        newNode = mainnode_mem + NODE_COUNT;
    }
    else{
        REMOVED_NODE_COUNT--;
        newNode = removednode_mem[REMOVED_NODE_COUNT];
    }
    NODE_COUNT++;

    /*the actual address of the node*/
    real_addr = (NODE*) (((long int)newNode) + pointer_adjust_node);

    
    if (mylist->num_of_nodes == 0){
        real_addr->item = item;
        mylist->currentNode = newNode;
        mylist->start = newNode;
        mylist->end = newNode;
    }
    else{
        /* making a new node and inserting 
        the item into the newnode*/
        real_addr->item = item; 

        /*adding links for new node*/
        real_addr->next = start;

        /*fixing links for old nodes*/
        start->prev = real_addr;

        /*updating old start to new end start 
        setting it to the current position*/
        mylist->start = newNode; 
        mylist->currentNode = mylist->start;
    }

    /* If nothing went wrong...*/
    if (real_addr->item == item){ 
        mylist->num_of_nodes++; /*increment number of nodes*/
        return 0;
    }
    else{
        return -1;
    }

}

/*adds list2 to the end of list1.
The current pointer is set to the current pointer of list1. 
List2 no longer exists after the operation.*/
void ListConcat( LIST* list1, LIST* list2){
    LIST* mylist1;
    LIST* mylist2;
    NODE* end1; /*Real address for end of list1*/
    NODE* start2; /*Real address for start of list2*/
    int k;

    if (list_shrunk){
        for(k = 0; k < LIST_COUNT; k++){
            if(real_addr_list[k*2]== list1){
                mylist1 = real_addr_list[(2*k)+1];
                k = LIST_COUNT; /* Breaks the loop*/
            }
        }
    }
    else{
        mylist1 = (LIST*) (((long int)list1) + pointer_adjust_list);
    }

    if (list_shrunk){
        for(k = 0; k < LIST_COUNT; k++){
            if(real_addr_list[k*2]== list2){
                mylist2 = real_addr_list[(2*k)+1];
                k = LIST_COUNT; /* Breaks the loop*/
            }
        }
    }
    else{
        mylist2 = (LIST*) (((long int)list2) + pointer_adjust_list);
    }

    end1 = (NODE*)(((long int)mylist1->end) + pointer_adjust_node);
    start2=(NODE*)(((long int)mylist2->start)+pointer_adjust_node);





    if ((mylist2->num_of_nodes == 0)){ 
        /* If only mylist1 exists, Nothing needs to be done*/
    }
    else if (mylist1->num_of_nodes == 0){
        /*If only mylist2 exists, 
        it basically copies mylist2 into mylist1*/
        mylist1->start = mylist2->start;
        mylist1->end = mylist2->end;
        mylist1->currentNode = mylist1->start;
        mylist1->num_of_nodes = mylist2->num_of_nodes;

    }
    else{
        end1->next = start2;
        start2->prev = end1;

        mylist1->end = mylist2->end;
        mylist1->num_of_nodes =mylist1->num_of_nodes+mylist2->num_of_nodes;
    }

    printf("Got to procedure ListConcat()\n");

    /*Set everything in list2 to 0*/
    mylist2->start = 0; 
    mylist2->end = 0;
    mylist2->currentNode = 0;
    mylist2->num_of_nodes = 0;

}
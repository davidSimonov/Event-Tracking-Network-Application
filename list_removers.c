/* Tommy Ojo, odo128, 11292921, A1-P3
David Simonov, das892, 11201273*/

/*WE ARE DOING THE BONUS, LOOK AT SECTION 6 IN THE DESIGN DOC FOR AN 
EXPLANATION*/

#include <stdlib.h>
#include <stdio.h>
#include <list.h>

extern int MIN_LISTS;
extern int MIN_NODES;

extern int NODE_COUNT;
extern int LIST_COUNT;

extern LIST* mainlist_mem;
extern NODE* mainnode_mem;
extern LIST* new_mainlist_mem;
extern NODE* new_mainnode_mem;

extern LIST* real_addr_list[1000];

extern int LIST_MULTIPLIER;
extern int NODE_MULTIPLIER;

extern int pointer_adjust_list;
extern int pointer_adjust_node;

extern LIST* removedlist_mem[5000];
extern NODE* removednode_mem[5000];

extern int REMOVED_NODE_COUNT;
extern int REMOVED_LIST_COUNT;
extern int list_shrunk;


/*Return current item and take it out of list.
 Make the next item the current one. */
void *ListRemove(LIST* list){
    NODE* next;
    NODE* curr;
    NODE* prev;
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

    if (mylist->num_of_nodes == 0){
        return NULL;
    }
    else if(mylist->num_of_nodes == 1){
        /*save the removed node*/
        removednode_mem[REMOVED_NODE_COUNT] = mylist->currentNode;
        REMOVED_NODE_COUNT++;

        mylist->end = NULL;
        mylist->start = NULL;
        mylist->currentNode = NULL;
        mylist->num_of_nodes--;
        NODE_COUNT--;

    
    }
    else if (mylist->currentNode == mylist->end){
        /*save the removed node*/
        removednode_mem[REMOVED_NODE_COUNT] = mylist->currentNode;
        REMOVED_NODE_COUNT++;

        prev = curr->prev;

        /* rearranging links to remove currentNode*/
        prev->next = NULL;

        curr->next = NULL;
        curr->prev = NULL;

        /* Setting currentNode to the next node*/
        mylist->currentNode = (NODE*)(((long int)prev) - pointer_adjust_node);
        mylist->end = mylist->currentNode;

        mylist->num_of_nodes--;
        NODE_COUNT--;

        
    }
    else if (mylist->currentNode == mylist->start){
        /*save the removed node*/
        removednode_mem[REMOVED_NODE_COUNT] = mylist->currentNode;
        REMOVED_NODE_COUNT++;

        next = curr->next;

        /* rearranging links to remove currentNode*/
        next->prev = NULL;
        curr->next = NULL;
        curr->prev = NULL;

        /* Setting currentNode to the next node*/
        mylist->start = (NODE*)(((long int)next) - pointer_adjust_node);
        mylist->currentNode = mylist->start;

        mylist->num_of_nodes--;
        NODE_COUNT--;

    }
    else{
        /*save the removed node*/
        removednode_mem[REMOVED_NODE_COUNT] = mylist->currentNode;
        REMOVED_NODE_COUNT++;

        next = curr->next;
        prev = curr->prev;

        /* rearranging links to remove currentNode*/
        prev->next = next;
        next->prev = prev;

        curr->next = NULL;
        curr->prev = NULL;

        /* Setting currentNode to the next node*/
        mylist->currentNode = (NODE*)(((long int)next) - pointer_adjust_node);

        mylist->num_of_nodes--;
        NODE_COUNT--;

    }

    return curr->item;
}

/* delete list. itemFree is a pointer to a routine that frees an item. 
 It should be invoked (within ListFree) as: (*itemFree)(itemToBeFreed);
 NEEDS A FUNCTION AS A PARAMETER*/
void ListFree(LIST* list, Itemfree itemFree){
    NODE* next;
    NODE* curr;
    LIST* prev_listmem;
    LIST* list_cursor;
    int i; /* used in for loops*/
    LIST* mylist;

    if (list_shrunk){
        for(i = 0; i < LIST_COUNT; i++){
            if(real_addr_list[i*2]== list){
                mylist = real_addr_list[(2*i)+1];
                i = LIST_COUNT; /* Breaks the loop*/
            }
        }
    }
    else{
        mylist = (LIST*) (((long int)list) + pointer_adjust_list);
    }

    curr = (NODE*) (((long int)mylist->start) + pointer_adjust_node);

    /* If list has not been Freed yet*/
    if(mylist->init == 1){
        if (mylist->num_of_nodes == 0){
            /* Dont iterate through the nodes if there are none*/
        }
        else{
            mylist->currentNode = mylist->start;
            while(curr != NULL){ 
            /* until and including the end of the mylist*/

            /*save the removed node*/
            removednode_mem[REMOVED_NODE_COUNT] = mylist->currentNode;
            REMOVED_NODE_COUNT++;
            next = curr->next;
            /* Deleting the  node*/
            (*itemFree)(curr);
            curr->next = NULL;
            curr->prev = NULL;

            curr = next;
            mylist->currentNode = (NODE*)(((long int)curr) - 
            pointer_adjust_node);

            mylist->num_of_nodes--;
            NODE_COUNT--;

            
            }
        }


        /*Setting start and end to NULL*/
        mylist->end = NULL;
        mylist->start = NULL;
        mylist->init = 0;

        /* Remove the mylist*/
        LIST_COUNT--;

        /*save the removed list*/
        removedlist_mem[REMOVED_LIST_COUNT] = list;
        REMOVED_LIST_COUNT++;

        /* If the used lists drop below half the size of list pool
        and list count is still >= than half of MIN_LISTS*/
        if (LIST_COUNT< MIN_LISTS*LIST_MULTIPLIER/2 && LIST_COUNT >= 
        MIN_LISTS/2){
            printf("Lists have too much extra space, reallocating less %d\n"
            , LIST_COUNT); 
            /* Halfing the available space*/
            LIST_MULTIPLIER = LIST_MULTIPLIER/2;
            /*Saving the spot for the second half of the old listmem*/
            prev_listmem = new_mainlist_mem +LIST_COUNT + 1;
            REMOVED_LIST_COUNT = 0;

            /*for every list that will be  cutoff with the realloc*/
            for (i = 0; i <= LIST_COUNT; i++){
                /* If list has been initialized with ListCreate() and not been 
                ListFreed yet*/
                if((prev_listmem+i)->init){ 
                    /*Using the removedlist_mem to hold all the lists that were 
                    cut off*/
                    (removedlist_mem[REMOVED_LIST_COUNT])->start = 
                    (prev_listmem + i)->start;
                    (removedlist_mem[REMOVED_LIST_COUNT])->end = (prev_listmem
                    +i)->end;
                    (removedlist_mem[REMOVED_LIST_COUNT])->currentNode = 
                    (prev_listmem + i)->currentNode;
                    (removedlist_mem[REMOVED_LIST_COUNT])->num_of_nodes = 
                    (prev_listmem + i)->num_of_nodes;
                    (removedlist_mem[REMOVED_LIST_COUNT])->og_addr = 
                    (prev_listmem + i)->og_addr;
                    REMOVED_LIST_COUNT++;
                } 
            }

            /*reallocing half the space*/
            new_mainlist_mem = realloc(new_mainlist_mem, 
            sizeof(LIST)*MIN_LISTS*LIST_MULTIPLIER);
            pointer_adjust_list = ((long int)new_mainlist_mem) - 
            ((long int)mainlist_mem);
            /*saves start position of new mainlist mem*/
            list_cursor = new_mainlist_mem;


            /*for every list in the list pool of the new listmem location
            MINUS the last one, which should always be left as the available 
            list spot*/
            for (i = 0; i < LIST_COUNT; i++){
                /*If the list has been deleted with ListFree()*/
                if((list_cursor)->init == 0){
                    /*If there are still any lists that have been cutoff*/
                    if(REMOVED_LIST_COUNT != 0){
                        REMOVED_LIST_COUNT--;
                        /*add list that was cutoff to that posiiton*/
                        list_cursor->start = (removedlist_mem
                        [REMOVED_LIST_COUNT])->start;
                        list_cursor->end = (removedlist_mem[REMOVED_LIST_COUNT])
                        ->end;
                        list_cursor->currentNode = (removedlist_mem
                        [REMOVED_LIST_COUNT])->currentNode;
                        list_cursor->num_of_nodes = (removedlist_mem
                        [REMOVED_LIST_COUNT])->num_of_nodes;
                        list_cursor->og_addr = (removedlist_mem
                        [REMOVED_LIST_COUNT])->og_addr;

                        /*Saving the original adress*/
                        real_addr_list[i*2] = list_cursor->og_addr;
    


                        /*Inserting the value for the REAL address of the LIST structure that correspond to the address of the 
                        variable that holds the list (key) */
                        real_addr_list[(i)*2+1] = list_cursor;


                        list_cursor->init = 1;
                    }
                    /*There should only be one available spot left in the pool 
                    of lists, and for my API to work, that HAS to be the last 
                    spot in the listmem, so I am moving the list in the last 
                    spot to another available  spot in listmem. 
                    This else loop should only run once */
                    else{
                        printf("The list_cursor is: %lx\n",(long int) 
                        list_cursor);
                        list_cursor->start = (new_mainlist_mem+LIST_COUNT)
                        ->start;
                        list_cursor->end = (new_mainlist_mem+LIST_COUNT)->end;
                        list_cursor->currentNode = (new_mainlist_mem+LIST_COUNT)
                        ->currentNode;
                        list_cursor->num_of_nodes = (new_mainlist_mem
                        +LIST_COUNT)->num_of_nodes;
                        list_cursor->og_addr = (new_mainlist_mem+LIST_COUNT)
                        ->og_addr;
                        list_cursor->init = 1;

                        /*Inserting the value for the REAL address of the LIST 
                        structure that correspond to the address of the 
                        variable that holds the list (key) */
                        real_addr_list[(i)*2+1] = list_cursor;
                        real_addr_list[(i)*2] = list_cursor->og_addr;

                        /*Now that the list is copied to the spot, we can 
                        delete the list */
                        (new_mainlist_mem+LIST_COUNT)->start = NULL;
                        (new_mainlist_mem+LIST_COUNT)->end = NULL;
                        (new_mainlist_mem+LIST_COUNT)->currentNode = NULL;
                        (new_mainlist_mem+LIST_COUNT)->num_of_nodes = 0;
                        (new_mainlist_mem+LIST_COUNT)->og_addr = NULL;
                        (new_mainlist_mem+LIST_COUNT)->init = 0;

                        /*remove address and corresponding real address from 
                        real address list*/
                        real_addr_list[(LIST_COUNT+1)*2] = 0;
                        real_addr_list[(LIST_COUNT+1)*2+1] = 0;

                    }
                    
                }
                else{ /*If List was not deleted and has stayed where it is*/

                    /*Inserting the value for the REAL address of the LIST 
                    structure that correspond to the address of the variable 
                    that holds the list (key) */
                    real_addr_list[(i)*2+1] = list_cursor;
                }
                list_cursor = list_cursor + 1; 
                /*Go to the next list in the pool of list nodes*/
            }

            list_shrunk = 1;
        
        
        }
    }
    else{
        printf("Cannot call ListFree on a list that has already been freed");
    }


    
}


/*Return last item and take it out of list. 
The current pointer shall be the new last item in the list.
 */
void *ListTrim(LIST* list){
    NODE* end;
    NODE* prev;
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

    if (mylist->num_of_nodes == 0){
        return NULL;
    }
    else if (mylist->num_of_nodes == 1){
        /*save the removed node*/
        removednode_mem[REMOVED_NODE_COUNT] = mylist->end;
        REMOVED_NODE_COUNT++;

        mylist->end = NULL;
        mylist->start = NULL;
        mylist->currentNode = NULL;
        mylist->num_of_nodes--;
        NODE_COUNT--;

        return end->item;
    }
    else{
        /*save the removed node*/
        removednode_mem[REMOVED_NODE_COUNT] =mylist->end;
        REMOVED_NODE_COUNT++;

        prev = end->prev;

        /* rearranging links to remove currentNode*/
        prev->next = NULL;
        end->prev = NULL;

        /* Setting currentNode to the next node*/
        mylist->currentNode = (NODE*)(((long int)prev) - pointer_adjust_node);
        mylist->end = mylist->currentNode;
        
        mylist->num_of_nodes--;
        NODE_COUNT--;

        return end->item;
    }
}


/*Return first item and take it out of list. 
The current pointer shall be the new first item in the list.
 */
void *ListBehead(LIST* list){
    NODE* start;
    NODE* next;
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

    if (mylist->num_of_nodes == 0){
        printf("Error in procedure ListBehead(): empty list\n");
        return NULL;
    }
    else if (mylist->num_of_nodes == 1){
        /*save the removed node*/
        removednode_mem[REMOVED_NODE_COUNT] = mylist->start;
        REMOVED_NODE_COUNT++;

        mylist->end = NULL;
        mylist->start = NULL;
        mylist->currentNode = NULL;
        mylist->num_of_nodes--;
        NODE_COUNT--;


        return start->item;
    }
    else{
        /*save the removed node*/
        removednode_mem[REMOVED_NODE_COUNT] =mylist->end;
        REMOVED_NODE_COUNT++;

        next = start->next;

        /* rearranging links to remove currentNode*/
        next->prev = NULL;
        start->next = NULL;

        /* Setting currentNode to the next node*/
        mylist->currentNode = (NODE*)(((long int)next) - pointer_adjust_node);
        mylist->start= mylist->currentNode;
        
        mylist->num_of_nodes--;
        NODE_COUNT--;

        return start->item;
    }
}
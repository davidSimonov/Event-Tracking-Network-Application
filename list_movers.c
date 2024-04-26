/* Tommy Ojo, odo128, 11292921, A1-P3
David Simonov, das892, 11201273*/

/*WE ARE DOING THE BONUS, LOOK AT SECTION 6 IN THE DESIGN DOC FOR AN 
EXPLANATION*/

#include <stdlib.h>
#include <stdio.h>
#include <list.h>

extern int pointer_adjust_list;
extern int pointer_adjust_node;
extern LIST* real_addr_list[1000];
extern int LIST_COUNT;
extern int list_shrunk;

/*returns the number of items in list.*/
int ListCount (LIST* list){
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

    return mylist->num_of_nodes;
}

/*returns a pointer to the first item in list and 
makes the first item the current item. */
void *ListFirst(LIST* list){
    LIST* mylist;
    NODE* curr;
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
    curr = (NODE*)(((long int)mylist->start) + pointer_adjust_node);

    if (mylist->num_of_nodes == 0){
        return NULL;
    }
    else{
        mylist->currentNode = mylist->start;
        return curr->item;
    }

}


/*returns a pointer to the last item in list and 
makes the last item the current one. */
void *ListLast(LIST* list){
    LIST* mylist;
    NODE* curr;
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
    curr = (NODE*)(((long int)mylist->end) + pointer_adjust_node);
        

    if (mylist->num_of_nodes == 0){
        return NULL;
    }
    else{
        mylist->currentNode = mylist->end;
        return curr->item;
    }
}

/*advances the list's current node by one, 
and returns a pointer to the new current item. 
If this operation attempts to advances the current item 
beyond the end of the list, a NULL pointer is returned. */
void *ListNext(LIST* list){
    NODE* next;
    LIST* mylist;
    NODE* curr;
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

    curr = (NODE*)(((long int)mylist->currentNode) + pointer_adjust_node);

    if (mylist->num_of_nodes == 0){
        return NULL;
    }
    else if(mylist->currentNode == mylist->end){
        return NULL;
    }
    else{
        next = curr->next;
        mylist->currentNode = (NODE*)(((long int)next) - pointer_adjust_node);
        return next->item;
    }
    
}

/* backs up the list's current node by one, and returns a pointer 
to the new current item. If this operation attempts to back up 
the current item beyond the start of the list, 
a NULL pointer is returned. */
void *ListPrev(LIST* list){
    NODE* prev;
    NODE* curr;
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
    curr = (NODE*)(((long int)mylist->currentNode) + pointer_adjust_node);

    if (mylist->num_of_nodes == 0){
        return NULL;
    }
    else if(mylist->currentNode == mylist->start){
        return NULL;
    }
    else{
        prev = curr->prev;
        mylist->currentNode = (NODE*)(((long int)prev) - pointer_adjust_node);
        return prev->item;
    }
}

/*returns a pointer to the current item in list. */
void *ListCurr(LIST* list){
    LIST* mylist;
    NODE* curr;
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
    curr = (NODE*)(((long int)mylist->currentNode) + pointer_adjust_node);

    if (mylist->num_of_nodes == 0){
        return NULL;
    }
    else{
        return curr->item;
    }
}

/*searches list starting at the current item until the end is reached or a
 match is found.  In this context, a match is determined by the comparator 
 parameter. This parameter is a pointer to a routine that takes as its first 
 argument an item pointer, and as its second argument comparisonArg. 
 Comparator returns 0 if the item and comparisonArg don't match 
(i.e. didn't find it), or 1 if they do (i.e. found it). Exactly what 
constitutes  a match is up to the implementor of comparator. If a match 
is found, the current  pointer is left at the matched item and the pointer 
to that item is returned.  If no match is found, the current pointer is left at 
the end of the list and a NULL pointer is returned. 
 NEEDS A FUNCTION AS A PARAMETER*/
void *ListSearch(LIST* list, Comparator comparator,void* comparisonArg){
    LIST* mylist;
    NODE* curr;
    NODE* end;
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
    curr = (NODE*)(((long int)mylist->start) + pointer_adjust_node);
    end = (NODE*)(((long int)mylist->end) + pointer_adjust_node);


    if (mylist->num_of_nodes == 0){ /* empty list*/
        return NULL;
    }
    else if ((*comparator)(end->item, comparisonArg)){
        /* If item is at the end of the list*/
        mylist->currentNode = (NODE*)(((long int)end) - pointer_adjust_node);
        return end->item;
    }
    else{
        mylist->currentNode = mylist->start;
        while(mylist->currentNode != mylist->end){
            /* until and including the end of the list*/
            if((*comparator)(curr->item, comparisonArg)){
                /* Match is found*/
                return curr->item;
            }
            else{
                curr = curr->next;
                mylist->currentNode = (NODE*)(((long int)curr) - 
                pointer_adjust_node);
            }

        }
    }

    return NULL;

}
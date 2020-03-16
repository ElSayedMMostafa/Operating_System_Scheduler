#include <stdio.h>
#include <stdlib.h>
 

struct Process_data 
{ 
   char name; 
   int priority; 
   int processing_time;  
   int starting_time;
};

typedef struct {
    int priority;
    struct Process_data data;
} node_t;

typedef struct {
    node_t *nodes;
    int len;
    int size;
} heap_t;
 
void push (heap_t *h, int priority, struct Process_data data) {
    if (h->len + 1 >= h->size) {
        h->size = h->size ? h->size * 2 : 4;
        h->nodes = (node_t *)realloc(h->nodes, h->size * sizeof (node_t));
    }
    int i = h->len + 1;
    int j = i / 2;
    while (i > 1 && h->nodes[j].priority > priority) {
        h->nodes[i] = h->nodes[j];
        i = j;
        j = j / 2;
    }
    h->nodes[i].priority = priority;
    h->nodes[i].data = data;
    h->len++;
}
 
struct Process_data pop (heap_t *h) {
    int i, j, k;
    if (!h->len) {
     struct Process_data NULL_Process_data = {'$',-1,-1,-1}; //the null value
        return NULL_Process_data  ; 
    }
    struct Process_data data = h->nodes[1].data;
 
    h->nodes[1] = h->nodes[h->len];
 
    h->len--;
 
    i = 1;
    while (i!=h->len+1) {
        k = h->len+1;
        j = 2 * i;
        if (j <= h->len && h->nodes[j].priority < h->nodes[k].priority) {
            k = j;
        }
        if (j + 1 <= h->len && h->nodes[j + 1].priority < h->nodes[k].priority) {
            k = j + 1;
        }
        h->nodes[i] = h->nodes[k];
        i = k;
    }
    return data;
}

// The next function is mine..
int peak_time (heap_t *h){
   if(h->len != 0) {return h->nodes[1].data.starting_time;}
   return -1;  
}

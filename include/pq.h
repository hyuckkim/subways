#ifndef PQ_H
#define PQ_H

#include "subways.h"

typedef struct {
    char station[MAX_NAME];
    Time time;
    char train_no[MAX_NAME];
    char route[MAX_NAME];
    int transfers;
    // For path reconstruction
    char prev_station[MAX_NAME];
} State;

typedef struct {
    State *data;
    int size;
    int capacity;
} PriorityQueue;

PriorityQueue* pq_create(int capacity);
void pq_push(PriorityQueue *pq, State s);
State pq_pop(PriorityQueue *pq);
int pq_is_empty(PriorityQueue *pq);
void pq_free(PriorityQueue *pq);

#endif

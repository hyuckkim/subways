#include "pq.h"
#include <stdlib.h>
#include <string.h>

PriorityQueue* pq_create(int capacity) {
    PriorityQueue *pq = malloc(sizeof(PriorityQueue));
    pq->data = malloc(sizeof(State) * capacity);
    pq->size = 0;
    pq->capacity = capacity;
    return pq;
}

void pq_push(PriorityQueue *pq, State s) {
    if (pq->size >= pq->capacity) return;
    
    int i = pq->size++;
    while (i > 0) {
        int p = (i - 1) / 2;
        // Primary sort: Time (Min-Heap)
        // Secondary sort: Transfers (Min-Heap)
        if (pq->data[p].time < s.time) break;
        if (pq->data[p].time == s.time && pq->data[p].transfers <= s.transfers) break;
        
        pq->data[i] = pq->data[p];
        i = p;
    }
    pq->data[i] = s;
}

State pq_pop(PriorityQueue *pq) {
    State res = pq->data[0];
    State last = pq->data[--pq->size];
    
    int i = 0;
    while (i * 2 + 1 < pq->size) {
        int child = i * 2 + 1;
        if (child + 1 < pq->size) {
            if (pq->data[child + 1].time < pq->data[child].time || 
               (pq->data[child + 1].time == pq->data[child].time && pq->data[child + 1].transfers < pq->data[child].transfers)) {
                child++;
            }
        }
        
        if (last.time < pq->data[child].time) break;
        if (last.time == pq->data[child].time && last.transfers <= pq->data[child].transfers) break;
        
        pq->data[i] = pq->data[child];
        i = child;
    }
    pq->data[i] = last;
    return res;
}

int pq_is_empty(PriorityQueue *pq) {
    return pq->size == 0;
}

void pq_free(PriorityQueue *pq) {
    free(pq->data);
    free(pq);
}

#include "subways.h"
#include "pq.h"
#include <limits.h>

typedef struct {
    Time time;
    int transfers;
    char prev_station[MAX_NAME];
    char train_no[MAX_NAME];
    char route[MAX_NAME];
} BestArrival;

// Simple station indexing
static char station_list[2048][MAX_NAME];
static int station_total = 0;

static int get_station_index(const char *name) {
    const char *canonical = get_canonical_name(name);
    for (int i = 0; i < station_total; i++) {
        if (strcmp(station_list[i], canonical) == 0) return i;
    }
    if (station_total < 2048) {
        strncpy(station_list[station_total], canonical, MAX_NAME);
        return station_total++;
    }
    return -1;
}

void search_route(const char *start_node, const char *end_node, Time start_time) {
    const char *start_canon = get_canonical_name(start_node);
    const char *end_canon = get_canonical_name(end_node);

    PriorityQueue *pq = pq_create(10000);
    
    // best_arrival[station_index]
    BestArrival best[2048];
    for (int i = 0; i < 2048; i++) {
        best[i].time = 2000; // Infinity (> 1440)
        best[i].transfers = INT_MAX;
    }

    int start_idx = get_station_index(start_canon);
    best[start_idx].time = start_time;
    best[start_idx].transfers = 0;

    State start_state;
    strncpy(start_state.station, start_canon, MAX_NAME);
    start_state.time = start_time;
    start_state.transfers = 0;
    start_state.train_no[0] = '\0';
    start_state.route[0] = '\0';
    start_state.prev_station[0] = '\0';

    pq_push(pq, start_state);

    int found = 0;
    while (!pq_is_empty(pq)) {
        State curr = pq_pop(pq);
        int curr_idx = get_station_index(curr.station);

        if (curr.time > best[curr_idx].time) continue;
        if (curr.time == best[curr_idx].time && curr.transfers > best[curr_idx].transfers) continue;

        if (strcmp(curr.station, end_canon) == 0) {
            found = 1;
            break; 
        }

        StationSchedule *sched = load_station_schedule(curr.station);
        if (!sched) continue;

        for (int i = 0; i < sched->departure_count; i++) {
            Departure *dep = &sched->departures[i];
            
            // Search window: only trains departing after current time, within 2 hours
            if (dep->time < curr.time) continue;
            if (dep->time > curr.time + 120) continue;

            int is_transfer = (strcmp(curr.train_no, dep->train_no) != 0);
            int new_transfers = curr.transfers + (is_transfer ? 1 : 0);

            Route *r = find_route(dep->terminal, curr.station);
            if (!r) continue;

            // Find current station in route to know subsequent stations
            int start_in_route = -1;
            for (int j = 0; j < r->station_count; j++) {
                if (strcmp(r->stations[j], curr.station) == 0) {
                    start_in_route = j;
                    break;
                }
            }
            if (start_in_route == -1) continue;

            for (int j = start_in_route + 1; j < r->station_count; j++) {
                const char *next_st = r->stations[j];
                Time arrival = find_arrival_time(next_st, dep->train_no);
                if (arrival == -1) continue;
                
                // Adjust for midnight crossing if necessary (not fully implemented here)
                if (arrival < dep->time) arrival += 1440;

                int next_idx = get_station_index(next_st);
                if (arrival < best[next_idx].time || 
                   (arrival == best[next_idx].time && new_transfers < best[next_idx].transfers)) {
                    
                    best[next_idx].time = arrival;
                    best[next_idx].transfers = new_transfers;
                    strncpy(best[next_idx].prev_station, curr.station, MAX_NAME);
                    strncpy(best[next_idx].train_no, dep->train_no, MAX_NAME);
                    strncpy(best[next_idx].route, dep->terminal, MAX_NAME);

                    State next_state;
                    strncpy(next_state.station, next_st, MAX_NAME);
                    next_state.time = arrival;
                    next_state.transfers = new_transfers;
                    strncpy(next_state.train_no, dep->train_no, MAX_NAME);
                    strncpy(next_state.route, dep->terminal, MAX_NAME);
                    strncpy(next_state.prev_station, curr.station, MAX_NAME);
                    pq_push(pq, next_state);
                }
            }
        }
    }

    if (found) {
        // Path reconstruction
        printf("Earliest arrival at %s: %02d:%02d (Transfers: %d)\n", 
               end_canon, (best[get_station_index(end_canon)].time % 1440) / 60, best[get_station_index(end_canon)].time % 60, 
               best[get_station_index(end_canon)].transfers - 1); // -1 because first boarding is counted as transfer in this logic
        
        // Detailed path print could go here
        char path_stack[128][MAX_NAME];
        char train_stack[128][MAX_NAME];
        Time time_stack[128];
        int top = 0;
        
        char cur[MAX_NAME];
        strncpy(cur, end_canon, MAX_NAME);
        while (cur[0] != '\0' && strcmp(cur, start_canon) != 0) {
            int idx = get_station_index(cur);
            strncpy(path_stack[top], cur, MAX_NAME);
            strncpy(train_stack[top], best[idx].train_no, MAX_NAME);
            time_stack[top] = best[idx].time;
            top++;
            strncpy(cur, best[idx].prev_station, MAX_NAME);
        }
        
        printf("Route:\n");
        printf("%s %02d:%02d 출발\n", start_canon, start_time / 60, start_time % 60);
        for (int i = top - 1; i >= 0; i--) {
            printf(" -> %s (%02d:%02d) 열차:%s\n", path_stack[i], (time_stack[i] % 1440) / 60, time_stack[i] % 60, train_stack[i]);
        }
    } else {
        printf("No path found from %s to %s\n", start_node, end_node);
    }

    pq_free(pq);
}

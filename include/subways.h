#ifndef SUBWAYS_H
#define SUBWAYS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME 64
#define MAX_STATIONS_PER_ROUTE 128
#define MAX_DEPARTURES 1024

// Time represented as minutes from 00:00 (0 to 1439)
typedef int Time;

typedef struct {
    char name[MAX_NAME];
    char target[MAX_NAME]; // The "canonical" name
} Alias;

typedef struct {
    char route_name[MAX_NAME];
    char stations[MAX_STATIONS_PER_ROUTE][MAX_NAME];
    int station_count;
} Route;

typedef struct {
    char train_no[MAX_NAME];
    Time time;
    char terminal[MAX_NAME]; // Terminal station/route name
} Departure;

typedef struct {
    char station_name[MAX_NAME];
    Departure departures[MAX_DEPARTURES];
    int departure_count;
} StationSchedule;

// For caching
extern StationSchedule **schedule_cache;
extern int cache_count;

// Function prototypes
Time parse_time(const char *time_str);
const char* get_canonical_name(const char *name);
int load_aliases(const char *filename);
int load_routes(const char *filename);
StationSchedule* load_station_schedule(const char *station_name);
Route* find_route(const char *name);
Time find_arrival_time(const char *station, const char *train_no);

void search_route(const char *start_node, const char *end_node, Time start_time);

#endif

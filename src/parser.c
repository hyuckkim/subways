#include "subways.h"
#include <ctype.h>

Alias *aliases = NULL;
int alias_count = 0;

Route *routes = NULL;
int route_count = 0;

StationSchedule **schedule_cache = NULL;
int cache_count = 0;

Time parse_time(const char *time_str) {
    int h, m;
    if (sscanf(time_str, "%d:%d", &h, &m) == 2) {
        return h * 60 + m;
    }
    // Handle HH format if necessary
    return atoi(time_str) * 60;
}

const char* get_canonical_name(const char *name) {
    for (int i = 0; i < alias_count; i++) {
        if (strcmp(aliases[i].name, name) == 0) {
            return aliases[i].target;
        }
    }
    return name;
}

int load_aliases(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return 0;

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        char *token = strtok(line, "; \t\n\r");
        if (!token) continue;

        char target[MAX_NAME];
        strncpy(target, token, MAX_NAME);

        while ((token = strtok(NULL, "; \t\n\r"))) {
            aliases = realloc(aliases, sizeof(Alias) * (alias_count + 1));
            strncpy(aliases[alias_count].name, token, MAX_NAME);
            strncpy(aliases[alias_count].target, target, MAX_NAME);
            alias_count++;
        }
    }
    fclose(fp);
    return alias_count;
}

int load_routes(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return 0;

    char line[2048];
    while (fgets(line, sizeof(line), fp)) {
        char *semicolon = strchr(line, ';');
        if (!semicolon) continue;

        routes = realloc(routes, sizeof(Route) * (route_count + 1));
        Route *r = &routes[route_count];
        
        *semicolon = '\0';
        strncpy(r->route_name, line, MAX_NAME);
        r->station_count = 0;

        char *stations_part = semicolon + 1;
        char *token = strtok(stations_part, " \t\n\r");
        while (token && r->station_count < MAX_STATIONS_PER_ROUTE) {
            strncpy(r->stations[r->station_count++], get_canonical_name(token), MAX_NAME);
            token = strtok(NULL, " \t\n\r");
        }
        route_count++;
    }
    fclose(fp);
    return route_count;
}

StationSchedule* load_station_schedule(const char *station_name) {
    // Check cache first
    for (int i = 0; i < cache_count; i++) {
        if (strcmp(schedule_cache[i]->station_name, station_name) == 0) {
            return schedule_cache[i];
        }
    }

    char path[256];
    snprintf(path, sizeof(path), "res/%s.txt", station_name);
    
    FILE *fp = fopen(path, "r");
    if (!fp) return NULL;

    StationSchedule *sched = calloc(1, sizeof(StationSchedule));
    strncpy(sched->station_name, station_name, MAX_NAME);

    char line[256];
    int current_hour = -1;
    while (fgets(line, sizeof(line), fp) && sched->departure_count < MAX_DEPARTURES) {
        // Handle hour headers (e.g., "05\t")
        if (isdigit(line[0]) && isspace(line[2])) {
            current_hour = atoi(line);
            continue;
        }

        char *colon = strchr(line, ':');
        if (!colon || colon == line) continue;

        // Extract time and handle potential HH:MM format
        // Some lines might only have MM if the hour is inherited
        Time t = -1;
        if (isdigit(*(colon-1)) && isdigit(*(colon-2))) {
            char time_buf[6];
            memcpy(time_buf, colon - 2, 5);
            time_buf[5] = '\0';
            t = parse_time(time_buf);
        } else if (current_hour != -1) {
            // This case handles some variation if HH is missing but current_hour is set
            int m = atoi(colon + 1);
            t = current_hour * 60 + m;
        }

        if (t == -1) continue;

        Departure *dep = &sched->departures[sched->departure_count];
        dep->time = t;
        
        // ... (rest of the existing parsing logic)
        char before[MAX_NAME] = {0};
        char after[MAX_NAME] = {0};
        
        int len_before = (colon - 2) - line;
        if (len_before >= MAX_NAME) len_before = MAX_NAME - 1;
        if (len_before > 0) {
            memcpy(before, line, len_before);
            before[len_before] = '\0';
            char *end = before + strlen(before) - 1;
            while(end > before && isspace(*end)) *end-- = '\0';
        }

        char *after_ptr = colon + 3;
        strncpy(after, after_ptr, MAX_NAME);
        char *end = after + strlen(after) - 1;
        while(end >= after && isspace(*end)) *end-- = '\0';

        if (strcmp(before, "회송") == 0 || strcmp(after, "회송") == 0) continue;

        if (isdigit(before[0]) || (before[0] >= 'A' && before[0] <= 'Z' && isdigit(before[1]))) {
            strncpy(dep->train_no, before, MAX_NAME);
            strncpy(dep->terminal, after, MAX_NAME);
        } else {
            strncpy(dep->terminal, before, MAX_NAME);
            strncpy(dep->train_no, after, MAX_NAME);
        }

        sched->departure_count++;
    }

    fclose(fp);
    
    // Add to cache
    schedule_cache = realloc(schedule_cache, sizeof(StationSchedule*) * (cache_count + 1));
    schedule_cache[cache_count++] = sched;
    
    return sched;
}

Time find_arrival_time(const char *station, const char *train_no) {
    StationSchedule *sched = load_station_schedule(station);
    if (!sched) return -1;
    for (int i = 0; i < sched->departure_count; i++) {
        if (strcmp(sched->departures[i].train_no, train_no) == 0) {
            return sched->departures[i].time;
        }
    }
    return -1;
}

Route* find_route(const char *name) {
    for (int i = 0; i < route_count; i++) {
        if (strcmp(routes[i].route_name, name) == 0) {
            return &routes[i];
        }
    }
    return NULL;
}

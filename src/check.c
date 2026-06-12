#include "subways.h"
#include <dirent.h>

int main() {
    load_aliases("alias.txt");
    printf("Loaded %d aliases\n", alias_count);
    
    load_routes("roads.txt");
    printf("Loaded %d routes\n", route_count);

    DIR *dir = opendir("res");
    if (!dir) {
        perror("Could not open res directory");
        return 1;
    }

    struct dirent *entry;
    int error_count = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        if (strstr(entry->d_name, ".txt") == NULL) continue;

        char station_name[MAX_NAME];
        strncpy(station_name, entry->d_name, sizeof(station_name));
        char *ext = strstr(station_name, ".txt");
        if (ext) *ext = '\0';

        StationSchedule *sched = load_station_schedule(station_name);
        if (!sched) continue;

        const char *canonical_station = get_canonical_name(station_name);

        char checked_terminals[MAX_DEPARTURES][MAX_NAME];
        int checked_count = 0;

        for (int i = 0; i < sched->departure_count; i++) {
            Departure *dep = &sched->departures[i];
            
            // Check if we already validated this terminal for this station
            int already_checked = 0;
            for (int k = 0; k < checked_count; k++) {
                if (strcmp(checked_terminals[k], dep->terminal) == 0) {
                    already_checked = 1;
                    break;
                }
            }
            if (already_checked) continue;

            // Mark as checked
            strncpy(checked_terminals[checked_count++], dep->terminal, MAX_NAME);

            Route *r = find_route(dep->terminal);
            
            if (!r) {
                printf("오류 #1: 노선 '%s' 가 없습니다. (%s.txt)\n", dep->terminal, station_name);
                error_count++;
                continue;
            }

            int found = 0;
            for (int j = 0; j < r->station_count; j++) {
                if (strcmp(r->stations[j], canonical_station) == 0) {
                    found = 1;
                    break;
                }
            }

            if (!found) {
                printf("오류 #2: 노선 '%s'에 %s가 없습니다. (%s.txt)\n", dep->terminal, canonical_station, station_name);
                error_count++;
            }
        }
        free(sched);
    }

    closedir(dir);
    printf("Integrity check finished with %d errors.\n", error_count);

    return 0;
}

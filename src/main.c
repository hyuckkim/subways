#include "subways.h"

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Usage: %s [start] [end] [HH:MM]\n", argv[0]);
        return 1;
    }

    load_aliases("alias.txt");
    load_routes("roads.txt");

    const char *start = argv[1];
    const char *end = argv[2];
    Time start_time = parse_time(argv[3]);

    if (start_time == -1) {
        printf("Invalid time format. Use HH:MM\n");
        return 1;
    }

    search_route(start, end, start_time);

    return 0;
}

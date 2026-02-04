#include <stdio.h>
#include <syslog.h>
#include <stdlib.h>


int main(int argc, char *argv[]) {
    openlog("writer", LOG_CONS, LOG_USER);
    if (argc != 3) {
        syslog(LOG_ERR, "Invalid number of arguments; Expected 2, got %d.", argc-1);
        syslog(LOG_ERR, "Usage: writer <file_path> <string>");
        exit(1);
    }
    FILE *fp = fopen(argv[1], "w");
    if (fp == NULL) {
        syslog(LOG_ERR, "Failed to open file %s for writing", argv[1]);
        exit(1);
    }
    syslog(LOG_DEBUG, "Writing %s to %s", argv[2], argv[1]);
    fprintf(fp, "%s", argv[2]);
    fclose(fp);
    return 0;
}
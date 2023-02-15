#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

#include "misc.h"
#include "fileDesc.h"

void fetchStats (char* path, char mask) { // probably going to delete so code quality doesn't matter
    if (mask & FSYM_STT) {
        struct stat buf;
        stat(path, &buf);
        printf("S_IN: %ld\n", buf.st_ino);
    }
    if (mask & FPHY_STT) {
        struct stat buf;
        lstat(path, &buf);
        printf("P_IN: %ld\n", buf.st_ino);
    }
    if (mask & FRDL_STT) {
        char bff[2048];
        readlink(path, bff, 2047);
        printf("FNM: %s\n", bff);
    }

    printf("\n");
}

int fetchProc () {
    DIR* proc = opendir("/proc");
    int c = 0;

    for (struct dirent* d = readdir(proc); d; d = readdir(proc)) {
        if (isNum(d->d_name)) { // uints are at most 20 chars
            char path[2048]; // 6 + 20 + 3 = 29 < 32 so it probably hopefully won't overflow
            sprintf(path, "/proc/%s/fd/", d->d_name);

            errno = 0;
            DIR* pDir = opendir(path); // process directory
            if (errno) {
                fprintf(stderr, "PID(%s) ERROR: %s\n", d->d_name, strerror(errno));
                continue;
            }

            for (struct dirent* f = readdir(pDir); f; f= readdir(pDir)) {
                if (isNum(f->d_name)) { // remove . / .. files
                    char fdPath[4096]; // same here
                    strcpy(fdPath, path); // fdPath > path so no overflow probably
                    strcat(fdPath, f->d_name); // 29 + 20 = 49 < 64
                    printf("PID: %s\nFD: %s\n", d->d_name, f->d_name);
                    fetchStats(fdPath, FPHY_STT | FRDL_STT);
                }
            }

            c++;
        }
    }

    return c;
}
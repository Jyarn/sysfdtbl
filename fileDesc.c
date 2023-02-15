#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "misc.h"
#include "fileDesc.h"

fdDsc* fetchStats (char* path) { // probably going to delete so code quality doesn't matter
    struct stat buf;
    stat(path, &buf);

    fdDsc* ret = malloc(sizeof(fdDsc));
    ret->symNode = buf.st_ino;

    lstat(path, &buf);
    ret->phyNode = buf.st_ino;

    readlink(path, ret->fName, 2047);
    ret->fName[2047] = '\0';

    ret->next = NULL;
    return ret;
}

int frPidPathFtFdInfo (char* pidPath, fdDsc* bff) {
/*
 * Provide ability to process all or specific pids
 * frPidPathFtFdInfo = From Process Id Path fetch File Descriptor Info
 * assumes bff is valid and is the end of the list
*/
    int c = 0;
    errno = 0;
    DIR* pDir = opendir(pidPath); // process directory

    if (errno) {
        return -1;
    }

    for (struct dirent* f = readdir(pDir); f; f = readdir(pDir)) {
        if (isNum(f->d_name)) { // remove . / .. files
            char fdPath[4096]; // same here
            strcpy(fdPath, pidPath); // fdPath > path so no overflow probably
            strcat(fdPath, f->d_name); // 29 + 20 = 49 < 64

            bff->next = fetchStats(fdPath);
            bff->fd = strtol(f->d_name, NULL, 10);
            bff = bff->next;
            c++;
        }
    }


    return c;
}

fdDsc* fetchProc (int thresh) {
/*
 * Main loop to be moved to main.c
 * Handle --threshold flag
*/

    DIR* proc = opendir("/proc");
    fdDsc temp;
    fdDsc* prev = &temp;

    for (struct dirent* d = readdir(proc); d; d = readdir(proc)) {
        if (isNum(d->d_name)) { // uints are at most 20 chars
            char path[2048]; // 6 + 20 + 3 = 29 < 32 so it probably hopefully won't overflow
            sprintf(path, "/proc/%s/fd/", d->d_name);

            if (frPidPathFtFdInfo(path, prev) == -1) {
                fprintf(stderr, "PID(%s) ERROR: %s\n", d->d_name, strerror(errno));
                continue;
            }

            prev = prev->next;
            prev->pid = strtol(d->d_name, NULL, 10);
        }
    }

    return temp.next;
}
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

int getDirSz (DIR** dir) {
    int c = 0;
    struct dirent* d;
    while ( (d = readdir(*dir)) ) { c += isNum(d->d_name); }
    rewinddir(*dir);
    return c;
}

void fetchStats (char* path, fdDsc* ret) { // probably going to delete so code quality doesn't matter
    struct stat buf;

    stat(path, &buf);
    ret->symNode = buf.st_ino;

    lstat(path, &buf);
    ret->phyNode = buf.st_ino;

    readlink(path, ret->fName, 2047);
    ret->fName[2047] = '\0';
}

int frPidPathFtFdInfo (char* pidPath, fdDsc** bff) {
/*
 * Provide ability to process all or specific pids
 * frPidPathFtFdInfo = From Process Id Path fetch File Descriptor Info
 * assumes bff is valid and is the end of the list
*/
    int c = 0;
    errno = 0;
    DIR* pDir = opendir(pidPath); // process directory
    if (errno) {
        fprintf(stderr, "ERROR (%s): %s\n", pidPath, strerror(errno));
        return -1;
    }

    int retSz = getDirSz(&pDir);
    if (retSz == -1) { return -1; }
    *bff = malloc(retSz * sizeof(fdDsc));

    for (struct dirent* f = readdir(pDir); f; f = readdir(pDir)) {
        if (isNum(f->d_name)) { // remove . / .. files
            char fdPath[4096]; // same here
            strcpy(fdPath, pidPath); // fdPath > path so no overflow probably
            strcat(fdPath, f->d_name); // 29 + 20 = 49 < 64

            fetchStats(fdPath, (*bff+c));
            (*bff+c)->fd = strtol(f->d_name, NULL, 10);

            c++;
        }
    }


    return c;
}

int fetchProc (pidFdDsc** bff) {
/*
 * Main loop to be moved to main.c
 * Handle --threshold flag
*/
    int c = 0;

    DIR* proc = opendir("/proc");
    
    int retSz = getDirSz(&proc);
    if (retSz == -1) { return -1; }
    *bff = malloc(retSz * sizeof(pidFdDsc));


    for (struct dirent* d = readdir(proc); d; d = readdir(proc)) {
        if (isNum(d->d_name)) { // uints are at most 20 chars
            char path[2048]; // 6 + 20 + 3 = 29 < 32 so it probably hopefully won't overflow
            sprintf(path, "/proc/%s/fd/", d->d_name);

            (*bff+c)->sz = frPidPathFtFdInfo(path, &((*bff+c)->fds));
            (*bff+c)->pid = strtol(d->d_name, NULL, 10);
            c++;
        }
    }

    return c;
}
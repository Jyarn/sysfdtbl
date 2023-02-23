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

    int r = readlink(path, ret->fName, 2047);
    ret->fName[r] = '\0';
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
        //fprintf(stderr, "ERROR (%s): %s\n", pidPath, strerror(errno));
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

    closedir(pDir);
    return c;
}

pidFdDsc* fetchAll (uid_t user, char printall) {
/*
 * Main loop to be moved to main.c
 * Handle --threshold flag
*/
    DIR* proc = opendir("/proc");
    struct stat uCheck;

    pidFdDsc* HEAD = NULL;
    pidFdDsc** write = &HEAD;

    for (struct dirent* d = readdir(proc); d; d = readdir(proc)) {
        if (isNum(d->d_name)) { // uints are at most 20 chars
            char path[2048]; // 6 + 20 + 3 = 29 < 32 so it probably hopefully won't overflow
            sprintf(path, "/proc/%s/", d->d_name);
            stat(path, &uCheck);

            if (printall || uCheck.st_uid == user) {
                strncat(path, "fd/", 2048-strlen(path));
                *write = malloc(sizeof(pidFdDsc) );
                (*write)->fds = NULL;
                (*write)->sz = frPidPathFtFdInfo(path, &(*write)->fds );
                (*write)->pid = strtol(d->d_name, NULL, 10);
                (*write)->next = NULL;
                write = &(*write)->next;
            }
        }
    }

    closedir(proc);
    return HEAD;
}

pidFdDsc* fetchSingle (int pid) {
    if (pid <= 0) {
        pid = getpid();
    }

    pidFdDsc* ret = malloc(1 * sizeof(pidFdDsc) );
    char path[2048];
    sprintf(path, "/proc/%d/fd/", pid);
    ret->sz = frPidPathFtFdInfo(path, &(ret->fds));
    ret->pid = pid;
    ret->next = NULL;
    return ret;
}

void destroyPidFdDsc (pidFdDsc* target) {
    if (target != NULL) {
        free(target->fds);
        destroyPidFdDsc(target->next);
        free(target);
    }
}
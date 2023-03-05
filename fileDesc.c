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
// get dir size, dir is assumed to be valid
    int c = 0;
    struct dirent* d;
    while ( (d = readdir(*dir)) ) { c += isNum(d->d_name); }
    rewinddir(*dir);
    return c;
}

void fetchStats (char* path, fdDesc* ret) {
/*
 * fetch stats for a given file descriptor and
 * puts them in ret
*/
    struct stat buf;

    stat(path, &buf);
    ret->phyNode = buf.st_ino;

    int r = readlink(path, ret->fName, 2047);
    ret->fName[r] = '\0';
}

int frPidPathFtFdInfo (char* pidPath, fdDesc** bff) {
/*
 * given a pidPath (/proc/[pid]/fd) loop through all file descriptors
 * and allocate a space in bff, then fill in all information in bff
 *
 * returns the size of bff, returns -1 if an error occured
*/
    int c = 0;
    errno = 0;
    DIR* pDir = opendir(pidPath); // process directory
    if (errno) {
        return -1;
    }

    int retSz = getDirSz(&pDir);
    if (retSz == -1) { return -1; }
    *bff = malloc(retSz * sizeof(fdDesc));

    for (struct dirent* f = readdir(pDir); f; f = readdir(pDir)) {
        if (isNum(f->d_name)) { // remove . / .. files
            char fdPath[4096];
            strcpy(fdPath, pidPath);
            strcat(fdPath, f->d_name);

            fetchStats(fdPath, (*bff+c));
            (*bff+c)->fd = strtol(f->d_name, NULL, 10);

            c++;
        }
    }

    closedir(pDir);
    return c;
}

pidFdDesc* fetchAll (uid_t user) {
/*
 * fetch all open file descriptors for the current user
 * if printall != 0 fetch all the open file descriptors
 *
 * returns a list pidFdDesc structs
*/
    DIR* proc = opendir("/proc");
    struct stat uCheck;

    pidFdDesc* HEAD = NULL;
    pidFdDesc** write = &HEAD;

    for (struct dirent* d = readdir(proc); d; d = readdir(proc)) {
        if (isNum(d->d_name)) {
            char path[2048];
            sprintf(path, "/proc/%s/", d->d_name);
            stat(path, &uCheck);

            // check if directory belongs to user
            if (uCheck.st_uid == user) {
                strncat(path, "fd/", 2048-strlen(path));

                // attach to tail of linked list
                *write = malloc(sizeof(pidFdDesc) );
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

pidFdDesc* fetchSingle (int pid) {
/*
 * fetch open file descriptors for a given pid
 * the pid is not checked if it is acessible
 * but if the pid <= 0, pid is set to the
 * current processes pid
*/
    pidFdDesc* ret = malloc(1 * sizeof(pidFdDesc) );
    char path[2048];
    sprintf(path, "/proc/%d/fd/", pid);
    ret->sz = frPidPathFtFdInfo(path, &(ret->fds));
    ret->pid = pid;
    ret->next = NULL;
    return ret;
}

void destroyPidFdDesc (pidFdDesc* target) {
/*
 * recursivley free alloc'd memory
*/
    if (target != NULL) {
        free(target->fds);
        destroyPidFdDesc(target->next);
        free(target);
    }
}
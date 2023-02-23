#ifndef __FILEDESC__
#define __FILEDESC__

#include <sys/types.h>

typedef struct s_fdDsc {
// file descriptor descriptor
    char              fName[2048];
    unsigned long int symNode;
    unsigned long int phyNode;
    unsigned int      fd;
} fdDsc;

typedef struct s_pidFdDsc {
// group all fdDscs by pid and store their sizes
    int                sz;
    int                pid;
    fdDsc*             fds;
    struct s_pidFdDsc* next;
} pidFdDsc;


pidFdDsc* fetchAll (uid_t user, char printall);
pidFdDsc* fetchSingle (int pid);
void destroyPidFdDsc (pidFdDsc* target);

#endif
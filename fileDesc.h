#ifndef __FILEDESC__
#define __FILEDESC__

#include <sys/types.h>

typedef struct s_fdDsc {
// file descriptor descriptor
    char              fName[2048]; // filename
    unsigned long int symNode;     // inode read by stat
    unsigned long int phyNode;     // inode read by lstat
    unsigned int      fd;          // file descriptor
} fdDsc;

typedef struct s_pidFdDsc {
// process id file descriptor descriptor
// group all fdDscs by pid and store their sizes
    int                sz;      // size of fds
    int                pid;     // the pid
    fdDsc*             fds;     // array of file descriptor descriptors
    struct s_pidFdDsc* next;    // next node
} pidFdDsc;


pidFdDsc* fetchAll (uid_t user, char printall);
pidFdDsc* fetchSingle (int pid);
void destroyPidFdDsc (pidFdDsc* target);

#endif
#ifndef __FILEDESC__
#define __FILEDESC__

#include <sys/types.h>

typedef struct s_fdDesc {
// file descriptor descriptor
    char              fName[2048]; // filename
    unsigned long int phyNode;     // inode read by stat
    unsigned int      fd;          // file descriptor
} fdDesc;

typedef struct s_pidFdDesc {
// process id file descriptor descriptor
// group all fdDescs by pid and store their sizes
    int                 sz;      // size of fds
    int                 pid;     // the pid
    fdDesc*             fds;     // array of file descriptor descriptors
    struct s_pidFdDesc* next;    // next node
} pidFdDesc;


pidFdDesc* fetchAll (uid_t user);
pidFdDesc* fetchSingle (int pid);
void destroyPidFdDesc (pidFdDesc* target);

#endif
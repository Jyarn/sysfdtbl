#ifndef __FILEDESC__
#define __FILEDESC__

typedef struct s_fdDsc {
// linked list of file description descriptions
    char              fName[2048];
    unsigned long int symNode;
    unsigned long int phyNode;
    struct s_fdDsc*   next;
    unsigned int      pid;
    unsigned int      fd;
} fdDsc;

fdDsc* fetchProc (int thresh);

#endif
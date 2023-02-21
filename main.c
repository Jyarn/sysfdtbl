#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "fileDesc.h"

void printInaccessible (int sz, pidFdDsc* p) {

}

void printThresh (int sz, pidFdDsc* p, int thresh) {

}

void printComposite (int sz, pidFdDsc* p, int thresh) {

}

void printSysWide (int sz, pidFdDsc* p, int thresh) {

}

void printVnodes (int sz, pidFdDsc* p, int thresh) {

}

void printINFO (int sz, pidFdDsc* t) {
    for (int i = 0; i < sz; i++) {
        if (t[i].sz != -1) {
            printf("PID: %d\n+--------+\n", t[i].pid);
            for (int j = 0; j < t[i].sz; j++) {
                printf("FD: %d\n S_IN: %ld\nP_IN: %ld\nFNM: %s\n\n",
                    t[i].fds[j].fd, 
                    t[i].fds[j].symNode,
                    t[i].fds[j].phyNode,
                    t[i].fds[j].fName
                );
            }

            printf("\n");
        }
    }
}

int main () {
    
    pidFdDsc* r;
    int sz = fetchAll(&r);
    printINFO(sz, r);
    destroyPidFdDsc(sz, r);

   /*
    pidFdDsc* r = fetchSingle(1);
    printINFO(1, r);
    destroyPidFdDsc(1, r);
   */
}
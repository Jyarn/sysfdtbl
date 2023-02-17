#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "fileDesc.h"

void printINFO (int sz, pidFdDsc* t) {
    for (int i = 0; i < sz; i++) {
        if (t[i].sz != -1) {
            printf("PID: %d\n+----------------+\n", t[i].pid);
            for (int j = 0; j < t[i].sz; j++) {
                printf("FD: %d\nS_IN: %ld\nP_IN: %ld\nFNM: %s\n\n", t[i].fds[j].fd, t[i].fds[j].symNode, t[i].fds[j].phyNode, t[i].fds[j].fName);
            }
        }
    }
}

int main () {
    pidFdDsc* r;
    int sz = fetchProc(0, &r);
    printINFO(sz, r);
}
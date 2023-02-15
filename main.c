#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "fileDesc.h"

void printINFO (fdDsc* t) {
    if (t) {
        printf(
            "PID: %d\nFD: %d\n S_IN: %ld\nP_IN: %ld\nFNM: %s\n\n",
            t->pid, t->fd, t->symNode, t->phyNode, t->fName
        );
        printINFO(t->next);
        free(t);
    }
}

int main () {
    printINFO(fetchProc(0));
}
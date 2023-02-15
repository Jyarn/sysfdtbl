#include <stdio.h>
#include <unistd.h>
#include "fileDesc.h"

int main () {
    printf("number of process: %d\n", fetchProc());
}
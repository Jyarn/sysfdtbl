#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>

#include "fileDesc.h"
#include "misc.h"

#define PRINT_INODES (1 << 0) // print inodes
#define PRINT_FLNAME (1 << 1) // print filename
#define PRINT_FLDESC (1 << 2) // print file descriptors
#define PRINT_PROCID (1 << 3) // print process id
#define PRINT_LNNUMS (1 << 4) // print line numbers

void printTable (pidFdDsc* in, char flags) {
    if (flags == 0) { return; }

    int titleBarLen = 0;
    printf("%-10s", " ");
    if (flags & PRINT_PROCID) { titleBarLen += printf("%-10s", "PID"); }
    if (flags & PRINT_FLDESC) { titleBarLen += printf("%-6s", "FD"); }
    if (flags & PRINT_INODES) { titleBarLen += printf("%-20s", "Inode"); }
    if (flags & PRINT_FLNAME) { titleBarLen += printf("%-50s", "Filename"); }

    printf("\n%8c", ' ');
    for (int i = 8; i < titleBarLen+10; i++) { printf("="); }
    int line = 1;

    while (in) {
        if (in->sz != -1) {
            for (int i = 0; i < in->sz; i++) {
                printf("\n");
                if (flags & PRINT_LNNUMS) { printf("%-10d", line); }
                else { printf("%10s", " "); }

                if (flags & PRINT_PROCID) { printf("%-10d", in->pid); }
                if (flags & PRINT_FLDESC) { printf("%-6d", in->fds[i].fd); }
                if (flags & PRINT_INODES) { printf("%-20ld", in->fds[i].symNode); }
                if (flags & PRINT_FLNAME) { printf("%-50s", in->fds[i].fName); }
                line++;
            }
        }


        in = in->next;
    }

    printf("\n%8c", ' ');
    for (int i = 8; i < titleBarLen+10; i++) { printf("="); }
    printf("\n\n\n");
}

void printThresh (pidFdDsc* in, int threshold) {
    int c = 0;

    printf("## Offending processes:");
    while (in) {
        if (in->sz != -1 && in->sz > threshold) {
            if (c % 10 == 0) { printf("\n"); }
            if (c % 10 != 0) { printf(", "); }
            printf("%d (%d)", in->pid, in->sz);
            c++;
        }

        in = in->next;
    }
}


int main (int argc, char** argv) {
    int qHead = 0;
    char tableQueue[(argc == 1) ? 1 : (argc-1)];

    const char composite = PRINT_PROCID | PRINT_FLDESC | PRINT_FLNAME | PRINT_INODES;
    const char sysWide   = PRINT_PROCID | PRINT_FLDESC | PRINT_FLNAME;
    const char vnodes    = PRINT_INODES;
    const char perProc   = PRINT_PROCID | PRINT_FLDESC;

    char printThreshold = 0;
    int threshold = 0;
    char printUsr = PRINT_LNNUMS;
    pid_t pid     = 0;

    tableQueue[0] = composite;

    for (int i = 1; i < argc; i++) {
        if ( !strcmp(argv[i], "--per-process")) { tableQueue[qHead++] = perProc; }
        else if ( !strcmp(argv[i], "--systemWide")) { tableQueue[qHead++] = sysWide; }
        else if ( !strcmp(argv[i], "--Vnodes")) { tableQueue[qHead++] = vnodes; }
        else if ( !strcmp(argv[i], "--composite")) { tableQueue[qHead++] = composite; }
        else if ( !strncmp(argv[i], "--threshold", 11)) {
            int off = 11;
            int argOff = i;
            if (argv[argOff][off] == '=') {
                off += 1;
            }
            else if (argv[argOff][off] == '\0') {
                if (argOff < argc - 1) {
                    argOff++;
                    off = 0;
                }
                else { continue; }
            }

            if (isNum(&argv[argOff][off]) || (argv[argOff][off] == '-' && isNum(&argv[argOff][off+1]))) {
                printThreshold = 1;
                threshold = atoi(&argv[argOff][off]);
            }
        }
    }

    if (isNum(argv[argc-1])) {
        printUsr = 0;
        pid = strtol(argv[argc-1], NULL, 10);
    }
    else if (!strcmp(argv[argc-1], "self")) {
        printUsr = 0;
        pid = -1;
    }

    pidFdDsc* r = NULL;
    if (printUsr) {
        struct passwd* p = getpwuid(geteuid() );
        printf(" >>> TARGETING USER: %s\n", p->pw_name);
        r = fetchAll(geteuid(), 0);
    }
    else {
        if (pid == -1) { printf(" >>> TARGETING SELF\n"); }
        else { printf(" >>> TARGETING PID: %d\n", pid);}
        r = fetchSingle(pid);
    }

    int a = qHead ? qHead : 1;
    for (int i = 0; i < a; i++) { // why doesn't my sanity reduce to an integer constant
        switch (tableQueue[i]) {
         case PRINT_PROCID | PRINT_FLDESC | PRINT_FLNAME | PRINT_INODES:
            printf("composite\n");
            break;

         case PRINT_PROCID | PRINT_FLDESC | PRINT_FLNAME:
            printf("sysWide\n");
            break;

         case PRINT_INODES:
            printf("vnodes\n");
            break;

         case PRINT_PROCID | PRINT_FLDESC:
            printf("perProc\n");
            break;
        }
    }

    a = qHead ? qHead : 1;
    for (int i = 0; i < a; i++) {
        printTable(r, tableQueue[i] | printUsr);
    }

    if (printThreshold) {
        printf("threshold = %d\n", threshold);
        printThresh(r, threshold);
    }
    else { printf("No Threshold\n"); }

    destroyPidFdDsc(r);
}
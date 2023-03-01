#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>

#include "fileDesc.h"
#include "misc.h"
#include "IOman.h"

#define PRINT_INODES (1 << 0) // print inodes
#define PRINT_FLNAME (1 << 1) // print filename
#define PRINT_FLDESC (1 << 2) // print file descriptors
#define PRINT_PROCID (1 << 3) // print process id
#define PRINT_LNNUMS (1 << 4) // print line numbers



void printTable (pidFdDsc* in, char flags, printMode outputMode) {
    if (flags == 0) { return; }

    int titleBarLen = 0;
    printOut(outputMode, "%-10s", " ");
    if (flags & PRINT_PROCID) { titleBarLen += printOut(outputMode, "%-10s", "PID"); }
    if (flags & PRINT_FLDESC) { titleBarLen += printOut(outputMode, "%-6s", "FD"); }
    if (flags & PRINT_INODES) { titleBarLen += printOut(outputMode, "%-20s", "Inode"); }
    if (flags & PRINT_FLNAME) { titleBarLen += printOut(outputMode, "%-50s", "Filename"); }

    printOut(outputMode, "\n%8c", ' ');
    for (int i = 8; i < titleBarLen+10; i++) { printOut(outputMode, "="); }
    int line = 1;

    while (in) {
        if (in->sz != -1) {
            for (int i = 0; i < in->sz; i++) {
                printOut(outputMode, "\n");
                if (flags & PRINT_LNNUMS) { printOut(outputMode, "%-10d", line); }
                else { printOut(outputMode, "%10s", " "); }

                if (flags & PRINT_PROCID) { printOut(outputMode, "%-10d", in->pid); }
                if (flags & PRINT_FLDESC) { printOut(outputMode, "%-6d", in->fds[i].fd); }
                if (flags & PRINT_INODES) { printOut(outputMode, "%-20ld", in->fds[i].symNode); }
                if (flags & PRINT_FLNAME) { printOut(outputMode, "%-50s", in->fds[i].fName); }
                line++;
            }
        }


        in = in->next;
    }

    printOut(outputMode, "\n%8c", ' ');
    for (int i = 8; i < titleBarLen+10; i++) { printOut(outputMode, "="); }
    printOut(outputMode, "\n\n\n");
}

void printThresh (printMode outputMode, pidFdDsc* in, int threshold) {
    int c = 0;

    printOut(outputMode, "## Offending processes:");
    while (in) {
        if (in->sz != -1 && in->sz > threshold) {
            if (c % 10 == 0) { printOut(outputMode, "\n"); }
            if (c % 10 != 0) { printOut(outputMode, ", "); }
            printOut(outputMode, "%d (%d)", in->pid, in->sz);
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
    const char vnodes    = PRINT_PROCID | PRINT_FLDESC | PRINT_INODES;
    const char perProc   = PRINT_PROCID | PRINT_FLDESC;

    char printThreshold = 0;
    int threshold = 0;
    char printUsr = PRINT_LNNUMS;
    pid_t pid     = 0;

    printMode outputMode = p_stdout;

    tableQueue[0] = composite;

    for (int i = 1; i < argc; i++) {
        if ( !strcmp(argv[i], "--per-process")) { tableQueue[qHead++] = perProc; }
        else if ( !strcmp(argv[i], "--systemWide")) { tableQueue[qHead++] = sysWide; }
        else if ( !strcmp(argv[i], "--Vnodes")) { tableQueue[qHead++] = vnodes; }
        else if ( !strcmp(argv[i], "--composite")) { tableQueue[qHead++] = composite; }
        else if ( !strcmp(argv[i], "--output_TXT")) { outputMode = p_text; }
        else if ( !strcmp(argv[i], "--output_binary")) { outputMode = p_binary; }

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
        printOut(outputMode, " >>> TARGETING USER: %s\n", p->pw_name);
        r = fetchAll(geteuid(), 0);
    }
    else {
        if (pid == -1) { printOut(outputMode, " >>> TARGETING SELF\n"); }
        else { printOut(outputMode, " >>> TARGETING PID: %d\n", pid);}
        r = fetchSingle(pid);
    }

    int a = qHead ? qHead : 1;
    for (int i = 0; i < a; i++) { // why doesn't my sanity reduce to an integer constant
        switch (tableQueue[i]) {
         case PRINT_PROCID | PRINT_FLDESC | PRINT_FLNAME | PRINT_INODES:
            printOut(outputMode, "composite\n");
            break;

         case PRINT_PROCID | PRINT_FLDESC | PRINT_FLNAME:
            printOut(outputMode, "sysWide\n");
            break;

         case PRINT_PROCID | PRINT_FLDESC | PRINT_INODES:
            printOut(outputMode, "vnodes\n");
            break;

         case PRINT_PROCID | PRINT_FLDESC:
            printOut(outputMode, "perProc\n");
            break;
        }
    }

    a = qHead ? qHead : 1;
    for (int i = 0; i < a; i++) {
        printTable(r, tableQueue[i] | printUsr, 0);
    }

    if (printThreshold) {
        printOut(outputMode, "threshold = %d\n", threshold);
        printThresh(outputMode, r, threshold);
    }
    else { printOut(outputMode, "No Threshold\n"); }

    destroyPidFdDsc(r);
}
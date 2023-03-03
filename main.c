#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>

#include "fileDesc.h"
#include "misc.h"
#include "IO.h"

#define PRINT_INODES (1 << 0) // print inodes
#define PRINT_FLNAME (1 << 1) // print filename
#define PRINT_FLDESC (1 << 2) // print file descriptors
#define PRINT_PROCID (1 << 3) // print process id
#define PRINT_LNNUMS (1 << 4) // print line numbers



void printTable (pidFdDsc* in, char flags, printMode outputMode, FILE* stream) {
    if (flags == 0) { return; }

    int titleBarLen = 0;
    printOut(stream, outputMode, "%-10s", " ");
    if (flags & PRINT_PROCID) { titleBarLen += printOut(stream, outputMode, "%-10s", "PID"); }
    if (flags & PRINT_FLDESC) { titleBarLen += printOut(stream, outputMode, "%-20s", "FD"); }
    if (flags & PRINT_INODES) { titleBarLen += printOut(stream, outputMode, "%-20s", "Inode"); }
    if (flags & PRINT_FLNAME) { titleBarLen += printOut(stream, outputMode, "%-50s", "Filename"); }

    printOut(stream, outputMode, "\n%8c", ' ');
    for (int i = 8; i < titleBarLen+10; i++) { printOut(stream, outputMode, "="); }
    int line = 1;

    while (in) {
        if (in->sz != -1) {
            for (int i = 0; i < in->sz; i++) {
                printOut(stream, outputMode, "\n");
                if (flags & PRINT_LNNUMS) { printOut(stream, outputMode, "%-10d", line); }
                else { printOut(stream, outputMode, "%10s", " "); }

                if (flags & PRINT_PROCID) { printOut(stream, outputMode, "%-10d", in->pid); }
                if (flags & PRINT_FLDESC) { printOut(stream, outputMode, "%-20d", in->fds[i].fd); }
                if (flags & PRINT_INODES) { printOut(stream, outputMode, "%-20ld", in->fds[i].symNode); }
                if (flags & PRINT_FLNAME) { printOut(stream, outputMode, "%-50s", in->fds[i].fName); }
                line++;
            }
        }


        in = in->next;
    }

    printOut(stream, outputMode, "\n%8c", ' ');
    for (int i = 8; i < titleBarLen+10; i++) { printOut(stream, outputMode, "="); }
    printOut(stream, outputMode, "\n\n");
}

void printThresh (FILE* stream, printMode outputMode, pidFdDsc* in, int threshold) {
    int c = 0;

    printOut(stream, outputMode, "## Offending processes:");
    while (in) {
        if (in->sz != -1 && in->sz > threshold) {
            if (c % 10 == 0) { printOut(stream, outputMode, "\n"); }
            if (c % 10 != 0) { printOut(stream, outputMode, ", "); }
            printOut(stream, outputMode, "%d (%d)", in->pid, in->sz);
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

    char printTxt = 0;
    char printBin = 0;

    tableQueue[0] = composite;

    for (int i = 1; i < argc; i++) {
        if ( !strcmp(argv[i], "--per-process")) { tableQueue[qHead++] = perProc; }
        else if ( !strcmp(argv[i], "--systemWide")) { tableQueue[qHead++] = sysWide; }
        else if ( !strcmp(argv[i], "--Vnodes")) { tableQueue[qHead++] = vnodes; }
        else if ( !strcmp(argv[i], "--composite")) { tableQueue[qHead++] = composite; }
        else if ( !strcmp(argv[i], "--output_TXT")) { printTxt = 1; }
        else if ( !strcmp(argv[i], "--output_binary")) { printBin = 1; }
        else if ( !strcmp(argv[i], "--notbl")) { qHead = -1; }

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

    if (isNum(argv[argc-1]) || (argv[argc-1][0] == '-' && isNum(&argv[argc-1][1]))) {
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
        printOut(stdout, p_stdout, " >>> TARGETING USER: %s\n", p->pw_name);
        r = fetchAll(geteuid(), 0);
    }
    else {
        if (pid  <= 0) { printOut(stdout, p_stdout, " >>> TARGETING SELF\n"); }
        else { printOut(stdout, p_stdout, " >>> TARGETING PID: %d\n", pid);}
        r = fetchSingle(pid);
    }

    int a = qHead ? qHead : 1;
    for (int i = 0; i < a; i++) {
        printTable(r, tableQueue[i] | printUsr, p_stdout, stdout);
    }

    if (printThreshold) {
        printThresh(stdout, p_stdout, r, threshold);
    }
    else { printOut(stdout, p_stdout, "No Threshold\n"); }

    if (printTxt) {
        FILE* txtOut = fopen(TXT_FLN, "w");
        printTable(r, composite, p_text, txtOut);
        fclose(txtOut);
    }
    if (printBin) {
        FILE* binOut = fopen(BIN_FLN, "wb");
        printTable(r, composite, p_binary, binOut);
        fclose(binOut);
    }

    destroyPidFdDsc(r);
}
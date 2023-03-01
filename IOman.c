#include <stdio.h>
#include <stdarg.h>

#include "IOman.h"


#define BIN_FLN "compositeTable.bin"
#define TXT_FLN "compositeTable.txt"

int printOut(printMode mode, const char* format, ...) {
    // https://en.cppreference.com/w/c/variadic
    va_list args;
    va_start(args, format);

    FILE* txtOut = stdout;
    int len;

    if (mode == p_stdout || mode == p_text) {
        if (mode == p_text) { txtOut = fopen(TXT_FLN, "a"); }
        len = vfprintf(txtOut, format, args);
    }
    else if (mode == p_binary) {
        txtOut = fopen(BIN_FLN, "a");

        char bff[2048];
        len = vsprintf(bff, format, args);
        fwrite(bff, sizeof(char), len+1, txtOut);
    }

    if (txtOut != stdout) { fclose(txtOut); }
    return len;
}

void fileInit (printMode mode) {
    FILE* fl = NULL;
    if (mode == p_binary) { fl = fopen(BIN_FLN, "w"); }
    else if (mode == p_text) { fl = fopen(TXT_FLN, "w"); }

    if (fl != NULL) { fclose(fl); }
}
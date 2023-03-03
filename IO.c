#include <stdio.h>
#include <stdarg.h>

#include "IO.h"

int printOut(FILE* txtOut, printMode mode, const char* format, ...) {
    // https://en.cppreference.com/w/c/variadic
    va_list args;
    va_start(args, format);

    int len;

    if (mode == p_stdout || mode == p_text) {
        len = vfprintf(txtOut, format, args);
    }
    else if (mode == p_binary) {
        char bff[2048];
        len = vsprintf(bff, format, args);
        fwrite(bff, sizeof(char), len, txtOut);
    }

    return len;
}
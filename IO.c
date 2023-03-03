#include <stdio.h>
#include <stdarg.h>

#include "IO.h"

int printOut(FILE* txtOut, printMode mode, const char* format, ...) {
/*
 * print to the indicate FILE* pointed to by txtOut, using the mode (binary or ascii)
 * printOut uses variadic functions to wrap printf and fwrite together
 * 
 * if mode == p_stdout or mode == p_text print to the txtOut, using vfprintf (basically printf)
 * if mode == p_binary use vsprintf to process the string and print to the file using fwrite
 * 
 * txtOut - File to print to (assumed to be opened in the correct mode and already opened/valid)
 * mode   - Specifies mode to print as (p_stdout is mostly redundant but was kept to not break things)
 * format - format string (see printf docs)
 * ...    - see printf docs    
*/
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
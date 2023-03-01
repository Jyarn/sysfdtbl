#ifndef __IOMAN__H
#define __IOMAN__H

typedef enum en_printMode {
    p_stdout,
    p_text,
    p_binary
} printMode;

int printOut (printMode mode, const char* format, ...);

#endif
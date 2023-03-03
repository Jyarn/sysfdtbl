#ifndef __IOMAN__H
#define __IOMAN__H

#define BIN_FLN "compositeTable.bin"
#define TXT_FLN "compositeTable.txt"

typedef enum {
    p_stdout,
    p_text,
    p_binary
} printMode;

int printOut(FILE* txtOut, printMode mode, const char* format, ...);

#endif
#include "misc.h"



int isNum (char* chk) {
    /*
    * Check if chk is a number
    * chk("70") == 1; chk("70sdsfd") == false; chk("slkdf98") == 0
    */

    for (int i = 0; chk[i]; i++) {
        if (chk[i] < '0' || chk[i] > '9') { return 0; }
    }

    return 1;
}

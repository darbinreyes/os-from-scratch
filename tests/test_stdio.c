#include "../include/stdio.h"
#include "../include/assert.h"
#include "../include/string.h"

void test_dtoa(void) {
    int d;
    char s[21];

    d = 257;
    dtoa(d, s);
    print(s);
    print("\n");
    assert(strcmp(s, "257") == 0);


    d = -587;
    dtoa(d, s);
    print(s);
    print("\n");
    assert(strcmp(s, "-587") == 0);
}
#include "../include/stdlib.h"
#include "../include/assert.h"

void test_atoi(void) {
    int i;
    char *str;
    int r;

    i = 587;
    str = "587";
    r = atoi(str);

    assert(i == r);

    i = -587;
    str = "-587";
    r = atoi(str);

    assert(i == r);

    i = 666;
    str = "777";
    r = atoi(str);

    assert(i == r);
}
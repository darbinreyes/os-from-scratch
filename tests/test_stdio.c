#include "../include/stdio.h"
#include "../include/assert.h"
#include "../include/string.h"

// @TODO Add all corner case tests.

void test_dtoa(void) {
    int d;
    char s[21];

    d = 0;
    _dtoa(d, s);
    assert(strcmp(s, "0") == 0);

    d = 257;
    _dtoa(d, s);
    assert(strcmp(s, "257") == 0);


    d = -587;
    _dtoa(d, s);
    assert(strcmp(s, "-587") == 0);

    d = -2147483647;
    _dtoa(d, s);
    // @IMPORTANT Bug when d = INT32_MIN = -2147483648. Underflow.
    assert(strcmp(s, "-2147483647") == 0);

    d = 2147483647;
    _dtoa(d, s);
    assert(strcmp(s, "2147483647") == 0);
}

void test_utoa(void) {
    unsigned int d;
    char s[21];


    d = 0;
    _utoa(d, s);
    assert(strcmp(s, "0") == 0);

    d = 4294967295;
    _utoa(d, s);
    assert(strcmp(s, "4294967295") == 0);
}

void test_xtoa(void) {
    int x;
    char s[21];

    x = 0xdeadbeef;
    _xtoa(x, 32, s, 0);
    assert(strcmp(s, "deadbeef") == 0);

    _xtoa(x, 32, s, 1);
    assert(strcmp(s, "DEADBEEF") == 0);

    x = 0x6666beef;
    _xtoa(x, 32, s, 0);
    assert(strcmp(s, "6666beef") == 0);

    _xtoa(x, 32, s, 1);
    assert(strcmp(s, "6666BEEF") == 0);

    unsigned long long int lx;

    lx = 0xdeadbeefcafebabe;
    _xtoa(lx, 64, s, 0);
    assert(strcmp(s, "deadbeefcafebabe") == 0);

    _xtoa(lx, 64, s, 1);
    assert(strcmp(s, "DEADBEEFCAFEBABE") == 0);

    lx = 0x6666beef7777babe;
    _xtoa(lx, 64, s, 0);
    assert(strcmp(s, "6666beef7777babe") == 0);

    _xtoa(lx, 64, s, 1);
    assert(strcmp(s, "6666BEEF7777BABE") == 0);

    lx = 18446744073709551615U;

    _xtoa(lx, 64, s, 0);
    assert(strcmp(s, "ffffffffffffffff") == 0);

    _xtoa(lx, 64, s, 1);
    assert(strcmp(s, "FFFFFFFFFFFFFFFF") == 0);
}

void test_all_stdio(void) {
    test_dtoa();
    test_utoa();
    test_xtoa();
}
#include "../include/stdio.h"
#include "../include/assert.h"
#include "../include/string.h"

void test_dtoa(void) {
    int d;
    char s[21];

    d = 257;
    dtoa(d, s);
    assert(strcmp(s, "257") == 0);


    d = -587;
    dtoa(d, s);
    assert(strcmp(s, "-587") == 0);
}

void test_xtoa(void) {
    int x;
    char s[21];

    x = 0xdeadbeef;
    xtoa(x, s, 0);
    assert(strcmp(s, "deadbeef") == 0);

    xtoa(x, s, 1);
    assert(strcmp(s, "DEADBEEF") == 0);

    x = 0x6666beef;
    xtoa(x, s, 0);
    assert(strcmp(s, "6666beef") == 0);

    xtoa(x, s, 1);
    assert(strcmp(s, "6666BEEF") == 0);
}

void test_llxtoa(void) {
    long long int x;
    char s[21];

    x = 0xdeadbeefcafebabe;
    llxtoa(x, s, 0);
    assert(strcmp(s, "deadbeefcafebabe") == 0);

    llxtoa(x, s, 1);
    assert(strcmp(s, "DEADBEEFCAFEBABE") == 0);

    x = 0x6666beef7777babe;
    llxtoa(x, s, 0);
    assert(strcmp(s, "6666beef7777babe") == 0);

    llxtoa(x, s, 1);
    assert(strcmp(s, "6666BEEF7777BABE") == 0);
}

void test_all_stdio(void) {
    test_dtoa();
    test_xtoa();
    test_llxtoa();
}
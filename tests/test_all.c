#include "test_assert.h"
#include "test_stdlib.h"
#include "test_stdio.h"
#include "../include/assert.h"

void test_all(void) {
    test_dtoa();
    test_atoi();
    test_assert();
    assert(0); // Marks the end of all tests.
}
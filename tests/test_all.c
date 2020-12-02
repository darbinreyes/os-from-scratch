#include "test_assert.h"
#include "test_stdlib.h"
#include "test_stdio.h"
#include "../include/assert.h"

void test_all(void) {
    test_all_stdio();
    test_all_stdlib();
    test_all_assert();
    print("All tests passed!\n");
    assert(0); // Marks the end of all tests.
}
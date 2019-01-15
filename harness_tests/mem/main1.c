// TODO - actually create this test
// basic_test is just to start a harness-based test

#include <test/test.h>
#include <uart/uart.h>

void basic_test(void) {
    ASSERT_EQ(1 + 1, 2);
}

test_t t1 = { .name = "Basic", .fn = basic_test };

test_t* suite[] = { &t1 };

int main() {
    run_tests(suite, sizeof(suite) / sizeof(suite[0]));
    return 0;
}

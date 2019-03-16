#include <assert.h>

typedef int (*setup_fun)(void);
typedef int (*teardown_fun)(void);

#define TEST(f,s,t,fx) {                \
    int test_result = 0;                \
    printf("Executing " #f "... ");     \
    assert(s((fx)) == 0);               \
    test_result = f(fx);                \
    assert(t((fx)) == 0);               \
    if (test_result == 0) {             \
        printf("OK!\n");                \
    } else {                            \
        printf("FAIL!\n");              \
    }                                   \
    assert(test_result == 0);           \
}

#define TEST_DEFAULT(f,fx) {                        \
    TEST(f, default_setup, default_teardown, fx);   \
}

#define ASSERT(x) {if (!(x)) return 1;}

#include "commit-object.h"
#include "test.h"
#include "util.h"

#include <assert.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

struct fixture {
    char *git_dir;
    char *object_hash;
};

static int default_setup(struct fixture *fx) {
    (void)fx;
    return 0;
}

static int default_teardown(struct fixture *fx) {
    (void)fx;
    return 0;
}

static int test_can_open_close(struct fixture *fx)
{
    struct commit_object obj = { 0 };
    ASSERT(commit_object_open(&obj, fx->git_dir, fx->object_hash) == 0);
    ASSERT(commit_object_close(&obj) == 0);
    return 0;
}


int main(int argc, char **argv) {
    (void) argc;
    (void) argv;

    /* Test on ourselves. This assumes the tests are executed from a child
     * directory of the Jävel source tree.. And that the referenced commit
     * exists - don't force push ;) */
    {
        struct fixture fx = {
            find_git_dir("."),
            "01ab57b643fdb569202168b5d2b6b3c58b3c1e6b"
            ""
        };

        TEST_DEFAULT(test_can_open_close, &fx);
        free(fx.git_dir);
    }
}

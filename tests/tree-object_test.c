#include "tree-object.h"
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
    struct tree_object obj = { 0 };
    ASSERT(tree_object_open(&obj, fx->git_dir, fx->object_hash) == 0);

    ASSERT(!strcmp(obj.leaves[0].mode, "100644"));
    ASSERT(!strcmp(obj.leaves[0].path, ".gitignore"));
    ASSERT(!strcmp(obj.leaves[0].hash, "81d7e6738afeeec0bd2b549157355d9c5a625fcc"));

    ASSERT(!strcmp(obj.leaves[1].mode, "100644"));
    ASSERT(!strcmp(obj.leaves[1].path, "LICENSE"));
    ASSERT(!strcmp(obj.leaves[1].hash, "46345a94843c63390fbbc94ff76eb4a4d278126f"));

    ASSERT(!strcmp(obj.leaves[2].mode, "100644"));
    ASSERT(!strcmp(obj.leaves[2].path, "README"));
    ASSERT(!strcmp(obj.leaves[2].hash, "a84a21911788fa07c245a1386a0ed037e38cf5a7"));

    ASSERT(!strcmp(obj.leaves[3].mode, "40000"));
    ASSERT(!strcmp(obj.leaves[3].path, "include"));
    ASSERT(!strcmp(obj.leaves[3].hash, "d4eb68b48519f6e8b69cfe0bc58e41a6dff5c39c"));

    ASSERT(!strcmp(obj.leaves[4].mode, "40000"));
    ASSERT(!strcmp(obj.leaves[4].path, "lib"));
    ASSERT(!strcmp(obj.leaves[4].hash, "a191dd3e9837513aa79ec30ccf8b157239797db5"));

    ASSERT(!strcmp(obj.leaves[5].mode, "100644"));
    ASSERT(!strcmp(obj.leaves[5].path, "meson.build"));
    ASSERT(!strcmp(obj.leaves[5].hash, "7bcfbe045a14fbe11bb65ff69ec897ff3f96d21b"));

    ASSERT(!strcmp(obj.leaves[6].mode, "40000"));
    ASSERT(!strcmp(obj.leaves[6].path, "src"));
    ASSERT(!strcmp(obj.leaves[6].hash, "9e872b38b7e8af16a6abdfc15bf181744c51e3c0"));

    ASSERT(tree_object_close(&obj) == 0);
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
            "27ab33d6995bec35fef5eb06fb353e5b598a3978",
        };

        TEST_DEFAULT(test_can_open_close, &fx);
        free(fx.git_dir);
    }
}

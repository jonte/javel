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
    char git_dir[PATH_MAX];
    char *object_hash;
    struct tree_object tree_obj;
};

static int default_setup(struct fixture *fx) {
    memset(&fx->tree_obj, 0, sizeof(fx->tree_obj));

    return 0;
}

static int default_teardown(struct fixture *fx) {
    tree_object_close(&fx->tree_obj);
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

static int test_can_serialize(struct fixture *fx) {
    uint8_t expected_data[] = {
        '1', '0', '0', '6', '4', '4', ' ', 't', 'e', 's', 't', '_', 'e', 'n',
        't', 'r', 'y' , 0x00, 0x9e, 0x87, 0x2b, 0x38, 0xb7, 0xe8, 0xaf, 0x16,
        0xa6, 0xab, 0xdf, 0xc1, 0x5b, 0xf1, 0x81, 0x74, 0x4c, 0x51, 0xe3, 0xc0,

        '1', '0', '0', '6', '4', '4', ' ', 't', 'e', 's', 't', '_', 'e', 'n',
        't', 'r', 'y', '2', 0x00, 0x9e, 0x87, 0x2b, 0x38, 0xb7, 0xe8, 0xaf,
        0x16, 0xa6, 0xab, 0xdf, 0xc1, 0x5b, 0xf1, 0x81, 0x74, 0x4c, 0x51, 0xe3,
        0xc0
    };

    uint8_t *actual_data = NULL;
    ssize_t buf_sz;

    ASSERT(tree_object_add_entry(&fx->tree_obj,
                                 0644,
                                 "9e872b38b7e8af16a6abdfc15bf181744c51e3c0",
                                 "test_entry",
                                 OBJECT_TYPE_BLOB) == 0);

    ASSERT(tree_object_add_entry(&fx->tree_obj,
                                 0644,
                                 "9e872b38b7e8af16a6abdfc15bf181744c51e3c0",
                                 "test_entry2",
                                 OBJECT_TYPE_BLOB) == 0);

    ASSERT((actual_data = tree_object_serialize(&fx->tree_obj, &buf_sz)) != 0);
    ASSERT(buf_sz == sizeof(expected_data));

    ASSERT(!memcmp(expected_data, actual_data, sizeof(expected_data)));
    free(actual_data);

    return 0;
}

static int test_can_add(struct fixture *fx) {
    ASSERT(tree_object_add_entry(&fx->tree_obj,
                                 0644,
                                 "9e872b38b7e8af16a6abdfc15bf181744c51e3c0",
                                 "test_entry",
                                 OBJECT_TYPE_BLOB) == 0);

    ASSERT(tree_object_add_entry(&fx->tree_obj,
                                 040000,
                                 "9e872b38b7e8af16a6abdfc15bf181744c51e3c0",
                                 "test_entry_dir",
                                 OBJECT_TYPE_TREE) == 0);

    ASSERT(tree_object_add_entry(&fx->tree_obj,
                                  040000,
                                  "9e872b38b7e8af16a6abdfc15bf181744c51e3c0",
                                  "test_entry_dir",
                                  OBJECT_TYPE_COMMIT) != 0);

    ASSERT(tree_object_add_entry(&fx->tree_obj,
                                 040000,
                                 "9e872b38b7e8af16a6abdfc15bf181744c51e3c0",
                                 "test_entry_dir",
                                 OBJECT_TYPE_TAG) != 0);
    return 0;
}

static int test_can_write(struct fixture *fx) {
    ASSERT(tree_object_add_entry(&fx->tree_obj,
                                 0644,
                                 "a559ebabec4b9bd15576851743692771640c12ea",
                                 "test_",
                                 OBJECT_TYPE_BLOB) == 0);

    free(tree_object_write(&fx->tree_obj, fx->git_dir));

    return 0;
}

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;

    /* Test on ourselves. This assumes the tests are executed from a child
     * directory of the Jävel source tree.. And that the referenced commit
     * exists - don't force push ;) */

    struct fixture fx = {{ 0 }};
    find_git_dir(".", fx.git_dir);
    fx.object_hash = "27ab33d6995bec35fef5eb06fb353e5b598a3978";

    TEST_DEFAULT(test_can_open_close, &fx);
    TEST_DEFAULT(test_can_serialize, &fx);
    TEST_DEFAULT(test_can_add, &fx);
    TEST_DEFAULT(test_can_write, &fx);

    return 0;
}

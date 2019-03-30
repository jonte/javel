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
    char git_dir[PATH_MAX];
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

static int test_can_serialize(struct fixture *fx)
{
    (void) fx;
    struct commit_object commit_obj = { 0 };
    struct object obj = { 0 };
    char parent[41] = {"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"};
    char tree[41] = {"BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"};
    struct identity author = {
        "Author",
        "author@email.com",
        1553287130,
        100
    };

    struct identity committer = {
        "Committer",
        "committer@email.com",
        1553287131,
        200
    };

    char *message = "Commit message";
    uint8_t *serialized_commit = NULL;
    ssize_t serialized_sz;
    char hash[41] = { 0 };

    ASSERT(commit_object_new(&commit_obj,
                             parent,
                             tree,
                             &author,
                             &committer,
                             message) == 0);

    serialized_commit = commit_object_serialize(&commit_obj, &serialized_sz);
    ASSERT(serialized_sz > 0);
    free(object_serialize(&obj,
                          OBJECT_TYPE_COMMIT,
                          serialized_commit,
                          serialized_sz,
                          &serialized_sz,
                          hash));
    ASSERT(serialized_sz > 0);
    commit_object_close(&commit_obj);
    free(serialized_commit);

    return 0;
}


int main(int argc, char **argv) {
    (void) argc;
    (void) argv;

    /* Test on ourselves. This assumes the tests are executed from a child
     * directory of the Jävel source tree.. And that the referenced commit
     * exists - don't force push ;) */
    struct fixture fx = {
        "",
        "01ab57b643fdb569202168b5d2b6b3c58b3c1e6b"
    };
    find_git_dir(".", fx.git_dir);

    TEST_DEFAULT(test_can_open_close, &fx);
    TEST_DEFAULT(test_can_serialize, &fx);
}

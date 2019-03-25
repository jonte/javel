#include "index.h"
#include "test.h"
#include "util.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

struct fixture {
    struct index idx;
    char git_dir[PATH_MAX];
};

static int default_setup(struct fixture *fx) {
    snprintf(fx->git_dir, sizeof(fx->git_dir), "/tmp/jvl_index_test_XXXXXX");
    mkdtemp(fx->git_dir);

    chdir(fx->git_dir);
    system("git init");
    system("echo 1 > test");
    system("echo 11 > test2");
    system("git add test test2");

    strncat(fx->git_dir, "/.git", sizeof(fx->git_dir));

    ASSERT(!index_open(&fx->idx, fx->git_dir));

    return 0;
}

static int default_teardown(struct fixture *fx) {
    char cmd[PATH_MAX * 2];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", fx->git_dir);
    system(cmd);

    ASSERT(!index_close(&fx->idx));
    return 0;
}

static int test_can_open_close(struct fixture *fx) {
    (void)fx;

    /* No-op - setup and teardown does all the work */

    return 0;
}

static int test_can_get_next_file(struct fixture *fx) {
    const struct index_file *idx_file = NULL;

    ASSERT(!index_next_file(&fx->idx, &idx_file));
    ASSERT(!strcmp(idx_file->path, "test"));
    ASSERT(idx_file->mode.permission == 0644);
    ASSERT(idx_file->mode.object_type == INDEX_FILE_REGULAR);
    ASSERT(idx_file->size == 2);

    ASSERT(!index_next_file(&fx->idx, &idx_file));
    ASSERT(!strcmp(idx_file->path, "test2"));
    ASSERT(idx_file->mode.permission == 0644);
    ASSERT(idx_file->mode.object_type == INDEX_FILE_REGULAR);
    ASSERT(idx_file->size == 3);

    return 0;
}

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;

    struct fixture fx = { 0 };

    TEST_DEFAULT(test_can_open_close, &fx);
    TEST_DEFAULT(test_can_get_next_file, &fx);

    return 0;
}

#include "test.h"
#include "util.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

struct fixture {
    char rm_command[2048];
    char test_dir[1024];
};

static int default_setup(struct fixture *fx) {
    int ret = 0;

    sprintf(fx->test_dir, "/tmp/util_test_XXXXXX");

    ret |= chdir(mkdtemp(fx->test_dir));
    ret |= system("mkdir .git");
    ret |= system("mkdir -p 1/2/3/4");
    return ret;
}

static int default_teardown(struct fixture *fx) {
    sprintf(fx->rm_command, "rm -rf %s", fx->test_dir);
    return system(fx->rm_command);
}

static int test_finds_existing_git_path(struct fixture *fx)
{
    char path[PATH_MAX];
    char expected_path[PATH_MAX];
    char *result_path = NULL;
    snprintf(path, PATH_MAX, "%s/1/2/3/4", fx->test_dir);
    snprintf(expected_path, PATH_MAX, "%s/.git", fx->test_dir);

    result_path = find_git_dir(path);
    ASSERT(strcmp(result_path, expected_path) == 0);
    free(result_path);
    return 0;
}

static int test_fails_finding_nonexisting_git_path(struct fixture *fx)
{
    char path[PATH_MAX];
    char *result_path = NULL;
    snprintf(path, PATH_MAX, "%s/..", fx->test_dir);

    result_path = find_git_dir(path);
    ASSERT(result_path == NULL);
    return 0;
}

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;

    struct fixture fx = {
        { 0 },
        { 0 }
    };

    TEST_DEFAULT(test_finds_existing_git_path, &fx);
    TEST_DEFAULT(test_fails_finding_nonexisting_git_path, &fx);
}

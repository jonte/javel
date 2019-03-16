#include "object.h"
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
    char write_file[PATH_MAX];
};

static const mode_t default_mode =
    S_IRUSR | S_IWUSR | S_IXUSR |
    S_IRGRP |           S_IXGRP |
                        S_IXOTH;

static int default_setup(struct fixture *fx) {
    char tmpfile[] = "/tmp/jvl_write_file_XXXXXX";
    char dummy_text[] = "abcdefghijklmnopqrstuvwxyz";
    int fd;

    mkdtemp(tmpfile);
    strncpy(fx->write_file, tmpfile, PATH_MAX);
    strcat(fx->write_file, "/file");

    fd = open(fx->write_file, O_WRONLY | O_CREAT, default_mode);
    assert(write(fd, dummy_text, sizeof(dummy_text)) == sizeof(dummy_text));
    close(fd);

    return 0;
}

static int default_teardown(struct fixture *fx) {
    (void)fx;
    return 0;
}

static int test_can_open_close(struct fixture *fx)
{
    struct object obj;
    ASSERT(object_open(&obj, fx->git_dir, fx->object_hash, 0) == 0);
    ASSERT(object_close(&obj) == 0);
    return 0;
}

static int test_can_read(struct fixture *fx)
{
    struct object obj;
    uint8_t buf[128] = { 0 };
    int len;


    ASSERT(object_open(&obj, fx->git_dir, fx->object_hash, 0) == 0);
    while ((len = object_read(&obj, buf, sizeof(buf))) > 0) {
        ASSERT(len > 0);
        for (int i = 0; i < len; i++) {
#if 0 /* print contents of decompressed stream */
            printf("%c", buf[i]);
#endif
        }
    }
    ASSERT(len >= 0);
    ASSERT(object_close(&obj) == 0);

    return 0;
}

static int test_can_write(struct fixture *fx)
{
    struct object obj = { 0 };
    char *hash = 0;

    hash = object_write(&obj, OBJECT_TYPE_BLOB, fx->write_file, 1);

    ASSERT(hash);

    free(hash);

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
            "551c0596f1bb03f6a707fe2959a6ddc785963992",
            ""
        };

        TEST_DEFAULT(test_can_open_close, &fx);
        TEST_DEFAULT(test_can_read, &fx);
        free(fx.git_dir);
    }


    {
        struct fixture fx = {
            find_git_dir("."),
            "",
            ""
        };

        TEST_DEFAULT(test_can_write, &fx);
        free(fx.git_dir);
    }

}

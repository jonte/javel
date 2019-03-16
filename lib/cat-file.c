#include "cat-file.h"
#include "log.h"
#include "object.h"
#include "util.h"

#include <stdint.h>

int jvl_cat_file(const char *hash) {
    struct object obj;
    uint8_t buf[1024];
    ssize_t read_sz = 0;

    char *git_dir = find_git_dir(".");
    if (!git_dir) {
        ERROR("Not a git repository");
        return -1;
    }

    if (object_open(&obj, git_dir, hash, 0)) {
        return -1;
    }

    while ((read_sz = object_read(&obj, buf, sizeof(buf))) > 0) {
        for (ssize_t i = 0; i < read_sz; i++) {
            printf("%c", buf[i]);
        }
    }

    return 0;
}

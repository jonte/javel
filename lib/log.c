#include "commit-object.h"
#include "logging.h"
#include "logging.h"
#include "show.h"
#include "util.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int jvl_log(const char *hash) {
    struct commit_object obj = { 0 };
    char *next_hash = strdup(hash);

    char *git_dir = find_git_dir(".");
    if (!git_dir) {
        ERROR("Not a git repository");
        return -1;
    }

    while (next_hash) {
        if (commit_object_open(&obj, git_dir, next_hash)) {
            return -1;
        }

        jvl_show(next_hash);
        printf("\n");

        free(next_hash);
        next_hash = NULL;
        if (obj.parent) {
            next_hash = strdup(obj.parent);
        }
        commit_object_close(&obj);
    }

    free(git_dir);

    return 0;
}

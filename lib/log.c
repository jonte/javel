#include "commit-object.h"
#include "common.h"
#include "logging.h"
#include "log.h"
#include "show.h"
#include "util.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int jvl_log(int argc, char **argv) {
    struct commit_object obj = { 0 };
    char next_hash[REF_MAX];
    char git_dir[PATH_MAX];

    if (find_git_dir(".", git_dir)) {
        ERROR("Not a git repository");
        return -1;
    }

    switch (argc) {
        case 1:
            if (get_head(git_dir, next_hash)) {
                ERROR("HEAD does not exist");
                return -1;
            }
            break;
        case 2:
            strncpy(next_hash, argv[1], sizeof(next_hash));
            break;
        default:
            ERROR("Command '%s' failed: The only allowed option is HASH",
                  argv[0]);
            return -1;
    }

    while (1) {
        memset(&obj, 0, sizeof(obj));
        if (commit_object_open(&obj, git_dir, next_hash)) {
            return -1;
        }

        char *show_cmd[] = {"show", next_hash};
        jvl_show(sizeof(show_cmd) / sizeof(*show_cmd), show_cmd);
        printf("\n");

        if (obj.parent[0]) {
            strncpy(next_hash, obj.parent, REF_MAX);
        } else {
            break;
        }
        commit_object_close(&obj);
    }

    return 0;
}

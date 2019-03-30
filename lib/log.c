#include "commit-object.h"
#include "logging.h"
#include "log.h"
#include "show.h"
#include "util.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int jvl_log(int argc, char **argv) {
    struct commit_object obj = { 0 };
    char *next_hash = NULL;
    char *git_dir = find_git_dir(".");

    switch (argc) {
        case 1:
            next_hash = get_head(git_dir);
            break;
        case 2:
            next_hash = strdup(argv[1]);
            break;
        default:
            ERROR("Command '%s' failed: The only allowed option is HASH",
                  argv[0]);
            return -1;
    }

    if (!git_dir) {
        ERROR("Not a git repository");
        free(next_hash);
        return -1;
    }

    while (next_hash) {
        memset(&obj, 0, sizeof(obj));
        if (commit_object_open(&obj, git_dir, next_hash)) {
            free(git_dir);
            free(next_hash);
            return -1;
        }

        char *show_cmd[] = {"show", next_hash};
        jvl_show(sizeof(show_cmd) / sizeof(*show_cmd), show_cmd);
        printf("\n");

        free(next_hash);
        next_hash = NULL;
        if (obj.parent[0]) {
            next_hash = strdup(obj.parent);
        }
        commit_object_close(&obj);
    }

    free(git_dir);

    return 0;
}

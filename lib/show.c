#include "commit-object.h"
#include "common.h"
#include "logging.h"
#include "show.h"
#include "util.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int jvl_show(int argc, char **argv) {
    struct commit_object obj = { 0 };
    char git_dir[PATH_MAX];
    char hash[REF_MAX];

    if (find_git_dir(".", git_dir)) {
        ERROR("Not a git repository");
        return -1;
    }

    switch (argc) {
        case 1:
            if (get_head(git_dir, hash)) {
                ERROR("Failed to resolve HEAD");
                return -1;
            }
            break;
        case 2:
            strncpy(hash, argv[1], sizeof(hash));
            break;
        default:
            ERROR("Command '%s' failed: The only allowed option is HASH",
                  argv[0]);
            return -1;
    }

    if (commit_object_open(&obj, git_dir, hash)) {
        return -1;
    }

    printf("Commit: %s\n", hash);
    printf("Tree: %s\n", obj.tree);

    if (obj.parent[0]) {
        printf("Parent: %s\n", obj.parent);
    }

    printf("Author: %s <%s>\n", obj.author.name, obj.author.email);
    printf("Committer: %s <%s>\n", obj.committer.name, obj.committer.email);
    printf("\n    ");
    for (char *c = obj.message; *c != '\0'; c++) {
        if (*c == '\n') {
            printf("\n    ");
        } else {
            putchar(*c);
        }
    }

    commit_object_close(&obj);

    return 0;
}

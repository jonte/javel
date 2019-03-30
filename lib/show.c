#include "commit-object.h"
#include "show.h"
#include "logging.h"
#include "util.h"

#include <stdint.h>
#include <stdlib.h>

static void usage(const char *cmd_name) {
    ERROR("Command '%s' failed: The only allowed parameter is HASH", cmd_name);
}

int jvl_show(int argc, char **argv) {
    if (argc != 2) {
        usage(argv[0]);
        return -1;
    }

    const char *hash = argv[1];
    struct commit_object obj = { 0 };

    char *git_dir = find_git_dir(".");
    if (!git_dir) {
        ERROR("Not a git repository");
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

    free(git_dir);

    return 0;
}

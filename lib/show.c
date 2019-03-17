#include "commit-object.h"
#include "show.h"
#include "logging.h"
#include "util.h"

#include <stdint.h>
#include <stdlib.h>

int jvl_show(const char *hash) {
    struct commit_object obj = { 0 };

    char *git_dir = find_git_dir(".");
    if (!git_dir) {
        ERROR("Not a git repository");
        return -1;
    }

    if (commit_object_open(&obj, git_dir, hash)) {
        return -1;
    }

    printf("commit: %s\n", hash);
    printf("tree: %s\n", obj.tree);
    printf("parent: %s\n", obj.parent);
    printf("author: %s\n", obj.author);
    printf("committer: %s\n", obj.committer);
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

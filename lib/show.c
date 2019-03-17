#include "commit-object.h"
#include "show.h"
#include "log.h"
#include "util.h"

#include <stdint.h>

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

    printf("tree: %s\n", obj.tree);
    printf("parent: %s\n", obj.parent);
    printf("author: %s\n", obj.author);
    printf("committer: %s\n", obj.committer);
    printf("\n%s", obj.message);

    commit_object_close(&obj);

    return 0;
}

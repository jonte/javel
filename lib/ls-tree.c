#include "tree-object.h"
#include "logging.h"
#include "show.h"
#include "util.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int jvl_ls_tree(int argc, char **argv) {
    if (argc != 2) {
        ERROR("Command '%s' failed: The only allowed parameter is HASH",
              argv[0]);
        return -1;
    }

    const char *hash = argv[1];
    struct tree_object obj = { 0 };

    char git_dir[PATH_MAX];
    if (find_git_dir(".", git_dir)) {
        ERROR("Not a git directory");
    }

    if (tree_object_open(&obj, git_dir, hash)) {
        return -1;
    }

    for (size_t i = 0; i < obj.num_leaves; i++) {
        struct object inner = { 0 };
        struct tree_leaf *leaf = &obj.leaves[i];

        if (object_open(&inner, git_dir, leaf->hash, 0)) {
            tree_object_close(&obj);
            ERROR("Referenced object %s can't be opened", leaf->hash);
            return -1;
        }

        const char *type = object_type_string(inner.type);
        printf("%06d %s %s\t %s\n", atoi(leaf->mode),
               type, leaf->hash, leaf->path);

        object_close(&inner);
    }

    tree_object_close(&obj);

    return 0;
}

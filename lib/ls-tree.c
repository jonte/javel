#include "tree-object.h"
#include "logging.h"
#include "show.h"
#include "util.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int jvl_ls_tree(const char *hash) {
    struct tree_object obj = { 0 };

    char *git_dir = find_git_dir(".");

    if (tree_object_open(&obj, git_dir, hash)) {
        return -1;
    }

    for (size_t i = 0; i < obj.num_leaves; i++) {
        struct object inner = { 0 };
        struct tree_leaf *leaf = &obj.leaves[i];

        if (object_open(&inner, git_dir, leaf->hash, 0)) {
            tree_object_close(&obj);
            free(git_dir);
            ERROR("Referenced object %s can't be opened", leaf->hash);
            return -1;
        }

        const char *type = object_type_string(inner.type);
        printf("%06d %s %s\t %s\n", atoi(leaf->mode),
               type, leaf->hash, leaf->path);

        object_close(&inner);
    }

    tree_object_close(&obj);
    free(git_dir);

    return 0;
}

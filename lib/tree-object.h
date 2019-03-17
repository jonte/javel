#include "object.h"

struct tree_leaf {
    char *mode;
    char *path;
    char hash[41];
};

struct tree_object {
    struct object obj;
    struct tree_leaf *leaves;
    size_t num_leaves;
};

int tree_object_open(struct tree_object *obj,
                     const char *git_dir,
                     const char *hash);

int tree_object_close(struct tree_object *obj);

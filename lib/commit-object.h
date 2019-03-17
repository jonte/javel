#include "object.h"

struct commit_object {
    struct object obj;
    char *tree;
    char *parent;
    char *author;
    char *committer;
    char *message;
};

int commit_object_open(struct commit_object *obj,
                       const char *git_dir,
                       const char *hash);

int commit_object_close(struct commit_object *obj);
const char *commit_object_get_tree(struct commit_object *obj);
const char *commit_object_get_parent(struct commit_object *obj);
const char *commit_object_get_author(struct commit_object *obj);
const char *commit_object_get_committer(struct commit_object *obj);
const char *commit_object_get_message(struct commit_object *obj);

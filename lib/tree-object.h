#pragma once

#include "object.h"

struct tree_leaf {
    char mode[7];
    char path[PATH_MAX];
    char hash[41];
    uint8_t hash_raw[20];
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

int tree_object_write(struct tree_object *obj, const char *git_dir);

int tree_object_new(struct tree_object *obj);

int tree_object_add_entry(struct tree_object *obj,
                          mode_t mode,
                          const char *hash,
                          const char *path,
                          enum object_type type);

uint8_t *tree_object_serialize(const struct tree_object *obj, ssize_t *buf_sz);

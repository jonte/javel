#pragma once

#include "object.h"
#include "tree-object.h"

struct identity {
    char *name;
    char *email;
    time_t date;
    int16_t tz;
};

struct commit_object {
    struct object obj;
    char tree[41];
    char parent[41];
    struct identity author;
    struct identity committer;
    char *message;
};

int commit_object_open(struct commit_object *obj,
                       const char *git_dir,
                       const char *hash);

int commit_object_new(struct commit_object *obj,
                      const char parent[41],
                      const char tree[41],
                      const struct identity *author,
                      const struct identity *committer,
                      const char *message);

uint8_t *commit_object_serialize(const struct commit_object *obj,
                                 ssize_t *buf_sz);

int commit_object_close(struct commit_object *obj);
char *commit_object_write(struct commit_object *obj, const char *git_dir);

#include "commit-object.h"
#include "logging.h"

#include <stdlib.h>
#include <string.h>

#define BUF_SZ 1024 * 10

static int populate_fields(struct commit_object *obj) {
    char buf[BUF_SZ] = { 0 };
    int remaining = obj->obj.size;

    while (remaining > 0) {
        int read = 0;
        for (read = 0; read < BUF_SZ; read++) {
            if (object_read(&obj->obj, (uint8_t*)buf + read, 1) < 1) {
                break;
            }

            if (buf[read] == '\n') {
                buf[read] = '\0';
                break;
            }
        }

        if (!strncmp(buf, "tree", 4)) {
            obj->tree = strdup(buf + 5);
        } else if (!strncmp(buf, "author", 6)) {
            obj->author = strdup(buf + 7);
        } else if (!strncmp(buf, "parent", 6)) {
            obj->parent = strdup(buf + 7);
        } else if (!strncmp(buf, "committer", 9)) {
            obj->committer = strdup(buf + 10);
        } else if (buf[0] == '\0' && remaining > 0) {
            obj->message = calloc(remaining, sizeof(*obj->message));
            read += object_read(&obj->obj, (uint8_t *)obj->message, remaining);
            if (read < 0)
            {
                ERROR("Failed to read message");
                return -1;
            }

            return 0;
        }

        remaining -= read;
    }

    return 0;
}

int commit_object_open(struct commit_object *obj,
                       const char *git_dir,
                       const char *hash)
{
    if (object_open(&obj->obj, git_dir, hash, 0)) {
        return -1;
    }

    if (populate_fields(obj)) {
        return -1;
    }

    return 0;
}

int commit_object_close(struct commit_object *obj)
{
    if (object_close(&obj->obj)) {
        return -1;
    }
    free(obj->tree);
    obj->tree = NULL;

    free(obj->parent);
    obj->parent = NULL;

    free(obj->author);
    obj->author = NULL;

    free(obj->committer);
    obj->committer = NULL;

    free(obj->message);
    obj->message = NULL;

    return 0;
}

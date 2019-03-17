#include "commit-object.h"
#include "log.h"

#include <stdlib.h>
#include <string.h>

#define BUF_SZ 1024 * 10

static int populate_fields(struct commit_object *obj) {
    char buf[BUF_SZ] = { 0 };
    char *bufp = buf;
    int read;

    while ((read = object_read(&obj->obj, (uint8_t*)buf, sizeof(buf)-1)) > 0) {
        if (obj->obj.type != OBJECT_TYPE_COMMIT) {
            ERROR("Object is not a commit");
            return -1;
        }

        while (bufp - buf < read) {
            char *bufp_end = bufp;
            while (*bufp_end != '\n' && *bufp_end != '\0') bufp_end++;
            if (!strncmp(bufp, "tree", 4)) {
                obj->tree = strndup(bufp + 5, bufp_end - bufp - 5);
            } else if (!strncmp(bufp, "author", 6)) {
                obj->author = strndup(bufp + 7, bufp_end - bufp - 7);
            } else if (!strncmp(bufp, "parent", 6)) {
                obj->parent = strndup(bufp + 7, bufp_end - bufp- 7);
            } else if (!strncmp(bufp, "committer", 9)) {
                obj->committer = strndup(bufp + 10, bufp_end - bufp - 10);
            } else if (!strncmp(bufp, "\n", 1)) {
                obj->message = strdup(bufp + 1);
            }
            bufp = bufp_end + 1;
        }
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
    free(obj->parent);
    free(obj->author);
    free(obj->committer);
    free(obj->message);

    return 0;
}
const char *commit_object_get_tree(struct commit_object *obj);
const char *commit_object_get_parent(struct commit_object *obj);
const char *commit_object_get_author(struct commit_object *obj);
const char *commit_object_get_committer(struct commit_object *obj);
const char *commit_object_get_message(struct commit_object *obj);

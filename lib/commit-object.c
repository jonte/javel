#include "commit-object.h"
#include "logging.h"

#include <stdlib.h>
#include <string.h>

#define BUF_SZ 1024 * 10

static int populate_identity(struct identity *id, char *str) {
    char *name = strtok(str, "<>");
    char *email = strtok(NULL, "<>");
    char *date = strtok(NULL, " ");
    char *tz = strtok(NULL, " ");

    id->name = strndup(name, strlen(name) - 1);
    id->email = strdup(email);
    id->date = strtol(date, NULL, 10);
    id->tz = strtol(tz, NULL, 10);

    return !(name && email && date && tz);
}

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
            memcpy(obj->tree, buf + 5, sizeof(obj->tree));
        } else if (!strncmp(buf, "author", 6)) {
            populate_identity(&obj->author, buf + 7);
        } else if (!strncmp(buf, "parent", 6)) {
            memcpy(obj->parent, buf + 7, sizeof(obj->parent));
        } else if (!strncmp(buf, "committer", 9)) {
            populate_identity(&obj->committer, buf + 10);
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

int commit_object_new(struct commit_object *obj,
                      const char parent[41],
                      const char tree[41],
                      const struct identity *author,
                      const struct identity *committer,
                      const char *message)
{
    memset(obj, 0, sizeof(struct commit_object));
    memcpy(obj->parent, parent, sizeof(obj->parent));
    memcpy(obj->tree, tree, sizeof(obj->tree));
    obj->author.name = strdup(author->name);
    obj->committer.name = strdup(committer->name);
    obj->author.email = strdup(author->email);
    obj->committer.email = strdup(committer->email);
    obj->author.date = author->date;
    obj->author.tz = author->tz;
    obj->committer.date = committer->date;
    obj->committer.tz = committer->tz;
    obj->message = strdup(message);

    return !(obj->author.email && obj->committer.email && obj->message);
}

uint8_t *commit_object_serialize(const struct commit_object *obj,
                                 ssize_t *buf_sz) {
    size_t obj_size =
        (sizeof(obj->parent) +
         sizeof(obj->tree) +
         strlen(obj->author.name) +
         strlen(obj->committer.name) +
         strlen(obj->author.email) +
         strlen(obj->committer.email) +
         sizeof(obj->author) +
         sizeof(obj->committer) +
         strlen(obj->message)) * 2;
    uint8_t *buf = malloc(obj_size);
    size_t buf_off = 0;
    char parent[sizeof(obj->parent) + 8] = { 0 };
    if (obj->parent[0]) {
        snprintf(parent, sizeof(parent), "parent %s\n", obj->parent);
    }

    buf_off += snprintf((char *)buf, obj_size,
                        "tree %s\n"
                        "%s"
                        "author %s <%s> %ld %c%04d\n"
                        "committer %s <%s> %ld %c%04d\n"
                        "\n"
                        "%s", obj->tree, parent, obj->author.name,
                        obj->author.email, obj->author.date,
                        obj->author.tz > 0 ? '+' : '-', obj->author.tz,
                        obj->committer.name,
                        obj->committer.email, obj->committer.date,
                        obj->author.tz > 0 ? '+' : '-', obj->committer.tz,
                        obj->message);

    *buf_sz = buf_off;
    return buf;
}

int commit_object_close(struct commit_object *obj) {
    if (object_close(&obj->obj)) {
        return -1;
    }

    free(obj->message);
    obj->message = NULL;

    free(obj->committer.name);
    obj->committer.name = NULL;

    free(obj->committer.email);
    obj->committer.email = NULL;

    free(obj->author.name);
    obj->author.name = NULL;

    free(obj->author.email);
    obj->author.email = NULL;

    return 0;
}

char *commit_object_write(struct commit_object *obj, const char *git_dir) {
    struct object obj_to_write = { 0 };
    ssize_t uncomp_buf_sz;
    ssize_t comp_buf_sz;
    uint8_t *comp_buf;
    char hash[41];

    uint8_t *uncomp_buf = commit_object_serialize(obj, &uncomp_buf_sz);
    comp_buf = object_serialize(&obj_to_write, OBJECT_TYPE_COMMIT, uncomp_buf,
                                uncomp_buf_sz, &comp_buf_sz, hash);

    if (object_write_to_file(&obj_to_write, git_dir, comp_buf, comp_buf_sz,
                             hash))
    {
        return NULL;
    }

    free(uncomp_buf);
    free(comp_buf);

    return strdup(hash);
}

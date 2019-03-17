#include "logging.h"
#include "sha1.h"
#include "tree-object.h"

#include <stdlib.h>
#include <string.h>

#define BUF_SZ PATH_MAX + 1024 /* Enough for any file name + sha + mode */

static int parse_leaf(uint8_t *buf, size_t buf_sz, struct tree_leaf *leaf) {
    size_t bufi = 0;

    /* Copy mode and path */
    for (bufi = 0; buf[bufi] != 0x20 && bufi < buf_sz; bufi++);
    buf[bufi++] = '\0';
    leaf->mode = strdup((char *)buf);
    leaf->path = strdup((char *)buf+bufi);

    /* Copy hash */
    for (; buf[bufi] != '\0' && bufi < buf_sz; bufi++);
    bufi++;
    SHA1DigestString(buf + bufi, leaf->hash);

    return bufi + 20;
}

static int populate_fields(struct tree_object *obj) {
    uint8_t buf[BUF_SZ] = { 0 };
    int total_read = 0;

    while (total_read < obj->obj.size) {
        int read = 0;
        for (read = 0; read < BUF_SZ; read++) {
            if (object_read(&obj->obj, (uint8_t*)buf + read, 1) < 1) {
                return 0;
            }

            /* Extract hash */
            if (buf[read] == '\0') {
                read++;
                if (object_read(&obj->obj, (uint8_t*)buf + read, 20) < 20) {
                    ERROR("Corrupt tree");
                    return -1;
                }

                break;
            }
        }

        obj->num_leaves++;
        int next_size = sizeof(*obj->leaves) * obj->num_leaves;
        obj->leaves = realloc(obj->leaves, next_size);

        parse_leaf(buf, read, &obj->leaves[obj->num_leaves - 1]);
        total_read += read;
    }

    return 0;
}

int tree_object_open(struct tree_object *obj,
                     const char *git_dir,
                     const char *hash)
{
    if (object_open(&obj->obj, git_dir, hash, 0)) {
        return -1;
    }

    if (obj->obj.type != OBJECT_TYPE_TREE) {
        ERROR("Object is not a tree");
        return -1;
    }

    if (populate_fields(obj)) {
        return -1;
    }

    return 0;
}

int tree_object_close(struct tree_object *obj)
{
    if (object_close(&obj->obj)) {
        return -1;
    }

    for (size_t i = 0; i < obj->num_leaves; i++) {
        struct tree_leaf *leaf = &obj->leaves[i];
        free(leaf->mode);
        leaf->mode = NULL;

        free(leaf->path);
        leaf->path = NULL;
    }

    free(obj->leaves);

    return 0;
}

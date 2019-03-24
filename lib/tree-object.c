#include "logging.h"
#include "sha1.h"
#include "tree-object.h"

#include <stdlib.h>
#include <string.h>

#define BUF_SZ (PATH_MAX + 1024) /* Enough for any file name + sha + mode */

static int parse_leaf(uint8_t *buf, size_t buf_sz, struct tree_leaf *leaf) {
    size_t bufi = 0;

    /* Copy mode and path */
    for (bufi = 0; buf[bufi] != 0x20 && bufi < buf_sz; bufi++);
    buf[bufi++] = '\0';
    memcpy(leaf->mode, buf, sizeof(leaf->mode));
    memcpy(leaf->path, buf+bufi, sizeof(leaf->path));

    /* Copy hash */
    for (; buf[bufi] != '\0' && bufi < buf_sz; bufi++);
    bufi++;
    SHA1DigestString(buf + bufi, leaf->hash);
    memcpy(leaf->hash_raw, buf + bufi, 20);

    return bufi + 20;
}

static int add_leaves(struct tree_object *obj, size_t num_new_leaves) {
        obj->num_leaves += num_new_leaves;
        int next_size = sizeof(*obj->leaves) * obj->num_leaves;
        obj->leaves = realloc(obj->leaves, next_size);

        return obj->leaves == NULL;
}

static int populate_fields_from_obj(struct tree_object *obj) {
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

        add_leaves(obj, 1);
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

    if (populate_fields_from_obj(obj)) {
        return -1;
    }

    return 0;
}

int tree_object_close(struct tree_object *obj)
{
    if (object_close(&obj->obj)) {
        return -1;
    }

    free(obj->leaves);

    return 0;
}

uint8_t *tree_object_serialize(const struct tree_object *obj, ssize_t *buf_sz) {
    uint8_t *buf = malloc(obj->num_leaves * (PATH_MAX + 60));
    size_t buf_off = 0;
    for (size_t i = 0; i < obj->num_leaves; i++) {
        struct tree_leaf *leaf = &obj->leaves[i];
        buf_off += snprintf((char*)buf + buf_off, PATH_MAX + 60, "%s %s",
                            leaf->mode, leaf->path);
        buf_off++;
        memcpy(buf + buf_off, leaf->hash_raw, 20);
        buf_off += 20;
    }

    *buf_sz = buf_off;

    return buf;
}

char *tree_object_write(struct tree_object *obj, const char *git_dir) {
    struct object obj_to_write = { 0 };
    ssize_t uncomp_buf_sz;
    ssize_t comp_buf_sz;
    uint8_t *comp_buf;
    char hash[41];

    uint8_t *uncomp_buf = tree_object_serialize(obj, &uncomp_buf_sz);
    comp_buf = object_serialize(&obj_to_write, OBJECT_TYPE_TREE, uncomp_buf,
                                uncomp_buf_sz, &comp_buf_sz, hash);

    if (object_write_to_file(&obj_to_write, git_dir, comp_buf, comp_buf_sz,
                             hash))
    {
        return NULL;
    }

    free(uncomp_buf);
    free(comp_buf);
    object_close(&obj_to_write);

    return strdup(hash);
}

int tree_object_add_entry(struct tree_object *obj,
                          mode_t mode,
                          const char *hash,
                          const char *path,
                          enum object_type type)
{
    struct tree_leaf *leaf;

    add_leaves(obj, 1);
    leaf = &obj->leaves[obj->num_leaves - 1];

    switch (type) {
        case OBJECT_TYPE_BLOB:
            snprintf(leaf->mode, sizeof(leaf->mode), "100%o", mode);
            break;
        case OBJECT_TYPE_TREE:
            snprintf(leaf->mode, sizeof(leaf->mode), "40000");
            break;
        default:
            ERROR("Unsupported object type for trees");
            return -1;
    }

    strncpy(leaf->path, path, sizeof(leaf->path));
    memcpy(leaf->hash, hash, sizeof(leaf->hash));
    SHA1Undigest(leaf->hash_raw, leaf->hash);

    return 0;
}

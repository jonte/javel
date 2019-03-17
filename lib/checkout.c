#include "checkout.h"
#include "commit-object.h"
#include "logging.h"
#include "show.h"
#include "tree-object.h"
#include "util.h"

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static int write_object(struct object *obj,
                        const char *mode,
                        const char *path,
                        const char *dir)
{
    uint8_t buf[1024];
    char full_path[PATH_MAX];
    int read;
    int fd;
    int mode_parsed = 0;

    if (!is_dir(dir)) {
        char cmd[PATH_MAX + 100];
        snprintf(cmd, PATH_MAX + 100, "mkdir -p %s", dir);
        if (system(cmd)) {
            ERROR("Failed to mkdir %s", dir);
            return -1;
        }
    }

    mode_parsed = strtol(mode + 3, NULL, 8);

    snprintf(full_path, PATH_MAX, "%s/%s", dir, path);
    fd = open(full_path, O_WRONLY | O_CREAT, mode_parsed);
    if (fd < 0) {
        ERROR("Failed to create %s: %s", full_path, strerror(errno));
        return -1;
    }

    while ((read = object_read(obj, buf, sizeof(buf))) > 0) {
        if (write(fd, buf, read) != read) {
            ERROR("Failed to write file: %s: %s", full_path, strerror(errno));
            return -1;
        }
    }

    return 0;
}

static int iterate_objects(struct tree_object *tree_obj,
                           const char *git_dir,
                           const char *dir)
{
    for (size_t i = 0; i < tree_obj->num_leaves; i++) {
        struct object obj = { 0 };
        struct tree_leaf *leaf = &tree_obj->leaves[i];

        if (object_open(&obj, git_dir, leaf->hash, 0)) {
            return -1;
        }

        switch (obj.type) {
            case OBJECT_TYPE_BLOB: {
                if (write_object(&obj, leaf->mode, leaf->path, dir)) {
                    object_close(&obj);
                    return -1;
                }
                break;
            } case OBJECT_TYPE_TREE: {
                char path[PATH_MAX] = { 0 };
                struct tree_object inner_tree_obj = { 0 };

                tree_object_open(&inner_tree_obj, git_dir, leaf->hash);

                strncat(path, dir, PATH_MAX);
                strncat(path, "/", PATH_MAX - 1);
                strncat(path, leaf->path, PATH_MAX);
                if (iterate_objects(&inner_tree_obj, git_dir, path)) {
                    ERROR("Failed to iterate tree");
                    return -1;
                }

                tree_object_close(&inner_tree_obj);
                break;
            } default:
                ERROR("Invalid object type");
                object_close(&obj);
                break;
        }

        object_close(&obj);
    }

    return 0;
}

int jvl_checkout(const char *hash, const char *dir) {
    struct commit_object commit_obj = { 0 };
    struct tree_object tree_obj = { 0 };

    char *git_dir = find_git_dir(".");

    if (commit_object_open(&commit_obj, git_dir, hash)) {
        return -1;
    }

    if (tree_object_open(&tree_obj, git_dir, commit_obj.tree)) {
        return -1;
    }

    if (iterate_objects(&tree_obj, git_dir, dir)) {
        ERROR("Failed to checkout");
        return -1;
    }


    tree_object_close(&tree_obj);
    commit_object_close(&commit_obj);
    free(git_dir);

    return 0;
}

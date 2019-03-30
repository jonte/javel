#include "index.h"
#include "logging.h"
#include "object.h"
#include "sha1.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int jvl_status(int argc, char **argv) {
    if (argc != 1) {
        ERROR("Command '%s' failed: No arguments allowed",
              argv[0]);
        return -1;
    }
    const struct index_file *idx_file = NULL;
    struct index idx = { 0 };

    char git_dir[PATH_MAX];
    if (find_git_dir(".", git_dir)) {
        ERROR("Could not find .git directory");
    }

    if (index_open(&idx, git_dir)) {
        return -1;
    }

    while (!index_next_file(&idx, &idx_file)) {
        char full_path[PATH_MAX];
        if (find_in_root(git_dir, idx_file->path, full_path)) {
            ERROR("Unable to find %s", full_path);
        }

        struct object obj = { 0 };
        char *new_hash = object_write(&obj,
                                      OBJECT_TYPE_BLOB,
                                      full_path,
                                      0);
        object_close(&obj);
        uint8_t new_hash_raw[20];

        if (!new_hash) {
            break;
        }

        SHA1Undigest(new_hash_raw, new_hash);

        printf("%s", idx_file->path);
        if (memcmp(new_hash_raw, idx_file->hash, sizeof(new_hash_raw))) {
            printf(" - MODIFIED\n");
        } else {
            printf(" - ADDED\n");
        }
    }

    index_close(&idx);

    return 0;
}

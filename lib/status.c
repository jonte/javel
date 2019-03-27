#include "index.h"
#include "object.h"
#include "sha1.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int jvl_status() {
    const struct index_file *idx_file = NULL;
    struct index idx = { 0 };

    char *git_dir = find_git_dir(".");

    if (index_open(&idx, git_dir)) {
        return -1;
    }

    while (!index_next_file(&idx, &idx_file)) {
        struct object obj = { 0 };
        char *new_hash = object_write(&obj,
                                      OBJECT_TYPE_BLOB,
                                      idx_file->path,
                                      0);
        uint8_t new_hash_raw[20];
        SHA1Undigest(new_hash_raw, new_hash);

        printf("%s", idx_file->path);
        if (memcmp(new_hash_raw, idx_file->hash, sizeof(new_hash_raw))) {
            printf(" - MODIFIED\n");
        } else {
            printf("\n");
        }
    }

    index_close(&idx);
    free(git_dir);

    return 0;
}

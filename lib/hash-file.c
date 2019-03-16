#include "hash-file.h"
#include "log.h"
#include "object.h"
#include "util.h"

#include <stdint.h>
#include <stdlib.h>

int jvl_hash_file(const char *file) {
    struct object obj = { 0 };
    char *hash = 0;

    hash = object_write(&obj, OBJECT_TYPE_BLOB, file, 1);
    if (!hash) {
        return -1;
    }

    printf("%s\n", hash);

    free(hash);

    return 0;
}

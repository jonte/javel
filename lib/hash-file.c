#include "hash-file.h"
#include "logging.h"
#include "object.h"
#include "util.h"

#include <stdint.h>
#include <stdlib.h>

int jvl_hash_file(int argc, char **argv) {
    if (argc != 2) {
        ERROR("Command '%s' failed: The only allowed parameter is FILE",
              argv[0]);
        return -1;
    }

    const char *file = argv[1];
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

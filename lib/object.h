#include <stdint.h>
#include <sys/types.h>
#include <zlib.h>

#define OBJECT_READ_BUF_SZ 1024

enum object_type {
    OBJECT_TYPE_BLOB,
    OBJECT_TYPE_TREE,
    OBJECT_TYPE_TAG,
    OBJECT_TYPE_COMMIT,
    OBJECT_TYPE_UNKNOWN
};

struct object {
    int fd;
    z_stream strm;
    uint8_t comp_buf[OBJECT_READ_BUF_SZ];
    enum object_type type;
    ssize_t size;
};

int object_open(struct object *obj, const char *git_dir, const char *hash);
int object_close(struct object *obj);

ssize_t object_read(struct object *obj, uint8_t *buf, size_t count);
ssize_t object_write(struct object *obj, const uint8_t *buf, size_t count);

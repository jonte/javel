#pragma once

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
    z_stream strm_inf;
    z_stream strm_def;
    uint8_t comp_buf[OBJECT_READ_BUF_SZ];
    enum object_type type;
    ssize_t size;
};

int object_open(struct object *obj,
                const char *git_dir,
                const char *hash,
                int create);

int object_close(struct object *obj);

ssize_t object_read(struct object *obj, uint8_t *buf, size_t count);

char *object_write(struct object *obj,
                   enum object_type type,
                   const char *file,
                   int write_to_db);

const char *object_type_string(enum object_type type);

uint8_t *object_serialize(struct object *obj,
                          enum object_type type,
                          uint8_t *buffer_in,
                          ssize_t buffer_in_sz,
                          ssize_t *buffer_out_used,
                          char hash_out[41]);

int object_write_to_file(struct object *obj,
                         const char *git_dir,
                         const uint8_t *buffer_out,
                         ssize_t buffer_out_sz,
                         const char *digest_string);

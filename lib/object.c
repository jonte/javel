#include "limits.h"
#include "logging.h"
#include "object.h"
#include "util.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <sha1.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MIN_OBJECT_HEADER_SZ 100
#define ZLIB_COMPRESSION_LEVEL 0

static const mode_t default_mode =
    S_IRUSR | S_IWUSR | S_IXUSR |
    S_IRGRP |           S_IXGRP |
                        S_IXOTH;

static char *object_type_strings[] = {
    "blob",
    "tree",
    "tag",
    "commit",
    "unknown"
};

static int setup_zlib_inflate(struct object *obj) {
    obj->strm_inf.zalloc = Z_NULL;
    obj->strm_inf.zfree = Z_NULL;
    obj->strm_inf.opaque = Z_NULL;
    obj->strm_inf.avail_in = 0;
    obj->strm_inf.next_in = Z_NULL;
    if (inflateInit(&obj->strm_inf) != Z_OK) {
        return -1;
    }

    return 0;
}

static int setup_zlib_deflate(struct object *obj, int level) {
    obj->strm_def.zalloc = Z_NULL;
    obj->strm_def.zfree = Z_NULL;
    obj->strm_def.opaque = Z_NULL;
    if (deflateInit(&obj->strm_def, level) != Z_OK) {
        return -1;
    }

    return 0;
}

static int parse_object_header(struct object *obj,
                               const uint8_t *buf,
                               size_t count) {
    uint8_t *endptr = 0;

    if (!memcmp(buf, "blob", 4)) {
        obj->type = OBJECT_TYPE_BLOB;
    } else if (!memcmp(buf, "tree", 4)) {
        obj->type = OBJECT_TYPE_TREE;
    } else if (!memcmp(buf, "commit", 6)) {
        obj->type = OBJECT_TYPE_COMMIT;
    } else if (!memcmp(buf, "tag", 3)) {
        obj->type = OBJECT_TYPE_TAG;
    } else {
        ERROR("Unknown object type/not a header");
        return -1;
    }

    for (size_t i = 0; i < count; i++) {
        if (buf[i] == 0x20) {
            obj->size = strtol((char*)buf+i, (char**)&endptr, 10);
            if (endptr == buf+i) {
                ERROR("Invalid size field in object");
                return -1;
            } else {
                break;
            }
        }
    }

    if (!endptr || obj->type == OBJECT_TYPE_UNKNOWN || obj->size == -1) {
        ERROR("Unable to parse object header");
        return -1;
    }

    return endptr - buf;
}

const char *object_type_string(enum object_type type) {
    return object_type_strings[type];
}

int object_open(struct object *obj,
                const char *git_dir,
                const char *hash,
                int create)
{
    char path[PATH_MAX] = { 0 };
    char objects_dir[PATH_MAX];
    snprintf(objects_dir, PATH_MAX, "%s/objects/%c%c/", git_dir, hash[0],
             hash[1]);

    strncat(path, objects_dir, PATH_MAX);
    strncat(path, hash+2, PATH_MAX);

    if (!is_file(path) && !create) {
        ERROR("Object does not exist: %s", path);
        return -1;
    }

    if (!is_dir(objects_dir)) {
        if (mkdir(objects_dir, default_mode)) {
            ERROR("Failed ot create objects directory %s: %s", objects_dir,
                  strerror(errno));
            return -1;
        }
    }

    obj->fd = open(path, (create ? O_RDWR | O_TRUNC : O_RDONLY) | O_CREAT,
                   default_mode);
    if (obj->fd < 0) {
        ERROR("Unable to open %s: %s", path, strerror(errno));
        return -1;
    }

    /* Size and type unknown */
    obj->size = -1;
    obj->type = OBJECT_TYPE_UNKNOWN;

    setup_zlib_inflate(obj);

    /* Parse header */
    object_read(obj, NULL, 0);

    return 0;
}

int object_close(struct object *obj) {
    close(obj->fd);
    inflateEnd(&obj->strm_inf);
    deflateEnd(&obj->strm_def);

    return 0;
}

static ssize_t read_compressed_chunk(z_stream *strm, uint8_t *buf,
                                     size_t count)
{
    int ret = 0;

    strm->avail_out = count;
    strm->next_out = buf;

    while (ret != Z_STREAM_END && strm->avail_out > 0) {
        ret = inflate(strm, Z_NO_FLUSH);
        assert(ret != Z_STREAM_ERROR);
        switch (ret) {
            case Z_NEED_DICT:
                ERROR("Decompressing object failed");
                return -1;
            case Z_DATA_ERROR:
                ERROR("Decompressing object failed");
                return -1;
            case Z_MEM_ERROR :
                ERROR("Decompressing object failed");
                return -1;
        }

        if (ret == Z_BUF_ERROR) {
            break;
        }
    }

    return count - strm->avail_out;
}

ssize_t object_read(struct object *obj, uint8_t *buf, size_t count) {
    ssize_t status;
    ssize_t read_sz = 0;

    if (obj->strm_inf.avail_in == 0) {
        status = read(obj->fd, obj->comp_buf, OBJECT_READ_BUF_SZ);

        if (status < 0) {
            ERROR("Failed to read compressed object data: %s",
                  strerror(errno));
            return -1;
        }

        /* EOF */
        if (status == 0)
            return 0;

        obj->strm_inf.avail_in = (size_t)status;
        obj->strm_inf.next_in = obj->comp_buf;

    }

    if (obj->type == OBJECT_TYPE_UNKNOWN && obj->size == -1) {
        uint8_t header[MIN_OBJECT_HEADER_SZ];
        int header_end = 0;

        for (header_end = 0; header_end < MIN_OBJECT_HEADER_SZ; header_end++) {
            read_compressed_chunk(&obj->strm_inf, header + header_end, 1);

            if (header[header_end] == '\0') {
                break;
            }
        }

        if (header_end == MIN_OBJECT_HEADER_SZ) {
            ERROR("No object header found");
            return -1;
        }

        if (parse_object_header(obj, header, header_end) < 0) {
            return -1;
        }
    }

    read_sz = read_compressed_chunk(&obj->strm_inf, buf, count);

    return read_sz;
}

uint8_t *object_serialize(struct object *obj,
                          enum object_type type,
                          uint8_t *buffer_in,
                          ssize_t buffer_in_sz,
                          ssize_t *buffer_out_used,
                          char hash_out[41])
{
    SHA1_CTX context;
    uint8_t digest[20] = { 0 };
    uint8_t *buffer_out;
    char digest_string[41];
    int ret;
    uint8_t header[MIN_OBJECT_HEADER_SZ] = { 0 };
    ssize_t header_sz = 0;
    uint8_t *complete_buffer_in;
    ssize_t complete_buffer_in_sz;

    setup_zlib_deflate(obj, ZLIB_COMPRESSION_LEVEL);

    header_sz = snprintf((char*)header, MIN_OBJECT_HEADER_SZ, "%s %ld",
                         object_type_string(type), buffer_in_sz) + 1;

    complete_buffer_in_sz = header_sz + buffer_in_sz;
    complete_buffer_in = malloc(complete_buffer_in_sz);

    /* This should be calculated better ..*/
    ssize_t buffer_out_sz = (complete_buffer_in_sz + 8 + 4);
    if (buffer_in_sz < 0) {
        return NULL;
    }

    buffer_out = malloc(buffer_out_sz);
    if (!buffer_out) {
        ERROR("Failed to allocate memory for object compression");
        free(buffer_out);
        return NULL;
    }

    memcpy(complete_buffer_in, header, header_sz);
    memcpy(complete_buffer_in + header_sz, buffer_in, buffer_in_sz);

    obj->strm_def.avail_out = buffer_out_sz;
    obj->strm_def.next_out = buffer_out;

    obj->strm_def.avail_in = complete_buffer_in_sz;
    obj->strm_def.next_in = complete_buffer_in;

    ret = deflate(&obj->strm_def, Z_FINISH);
    assert(ret != Z_STREAM_ERROR);

    *buffer_out_used = buffer_out_sz - obj->strm_def.avail_out;

    SHA1Init(&context);
    SHA1Update(&context, complete_buffer_in, complete_buffer_in_sz);
    SHA1Final(digest, &context);
    SHA1DigestString(digest, digest_string);

    deflateEnd(&obj->strm_def);

    strncpy(hash_out, digest_string, 41);
    free(complete_buffer_in);

    return buffer_out;
}

int object_write_to_file(struct object *obj,
                         const char *git_dir,
                         const uint8_t *buffer_out,
                         ssize_t buffer_out_sz,
                         const char *digest_string)
{
    int ret = object_open(obj, git_dir, digest_string, 1);

    if (!ret) {
        if (write(obj->fd, buffer_out, buffer_out_sz) != buffer_out_sz) {
            ERROR("Failed to write object to file");
            return -1;
        }
    }

    return 0;
}

char *object_write(struct object *obj,
                   enum object_type type,
                   const char *file,
                   int write_to_db)
{
    uint8_t *buffer_in = NULL;
    uint8_t *buffer_out;
    int fd;
    char digest_string[41];
    ssize_t buffer_out_sz;
    ssize_t data_read;

    ssize_t buffer_in_sz = file_size(file);

    if (buffer_in_sz < 0) {
        return NULL;
    }

    fd = open(file, O_RDONLY);
    if (fd < 0) {
        ERROR("Can't open %s: %s", file, strerror(errno));
        return NULL;
    }

    buffer_in = malloc(buffer_in_sz * sizeof(*buffer_in));
    if (!buffer_in) {
        ERROR("Unable to allocate buffer");
        return NULL;
    }

    data_read = read(fd, buffer_in, buffer_in_sz);
    if (data_read != buffer_in_sz) {
        ERROR("Unable to read all of %s: %s", file, strerror(errno));
        return NULL;
    }

    buffer_out = object_serialize(obj, type, buffer_in, buffer_in_sz,
                                  &buffer_out_sz, digest_string);

    free(buffer_in);

    if (write_to_db) {
        char git_dir[PATH_MAX];
        if (find_git_dir(".", git_dir)) {
            ERROR("Not a git directory");
        }

        object_write_to_file(obj, git_dir, buffer_out, buffer_out_sz,
                             digest_string);
    }

    free(buffer_out);

    return strndup(digest_string, sizeof(digest_string));
}

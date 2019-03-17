#include "limits.h"
#include "log.h"
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
    obj->strm.zalloc = Z_NULL;
    obj->strm.zfree = Z_NULL;
    obj->strm.opaque = Z_NULL;
    obj->strm.avail_in = 0;
    obj->strm.next_in = Z_NULL;
    if (inflateInit(&obj->strm) != Z_OK) {
        return -1;
    }

    return 0;
}

static int setup_zlib_deflate(struct object *obj, int level) {
    obj->strm.zalloc = Z_NULL;
    obj->strm.zfree = Z_NULL;
    obj->strm.opaque = Z_NULL;
    if (deflateInit(&obj->strm, level) != Z_OK) {
        return -1;
    }

    return 0;
}

static int parse_object_header(struct object *obj,
                               const uint8_t *buf,
                               size_t count) {
    /* We haven't seen the type or size yet, so this should be the first
     * pass on the object */
    if (count < MIN_OBJECT_HEADER_SZ) {
        ERROR("The first read must be bigger than %d bytes to "
              "accomodate for the git object header",
              MIN_OBJECT_HEADER_SZ);
        return -1;
    }

    if (!memcmp(buf, "blob", 4)) {
        obj->type = OBJECT_TYPE_BLOB;
    } else if (!memcmp(buf, "tree", 4)) {
        obj->type = OBJECT_TYPE_TREE;
    } else if (!memcmp(buf, "commit", 6)) {
        obj->type = OBJECT_TYPE_COMMIT;
    } else if (!memcmp(buf, "tag", 3)) {
        obj->type = OBJECT_TYPE_TAG;
    } else {
        ERROR("Unknown object type");
        return -1;
    }

    for (int i = 0; i < MIN_OBJECT_HEADER_SZ; i++) {
        if (buf[i] == 0x20) {
            uint8_t *endptr = 0;
            obj->size = strtol((char*)buf+i, (char**)&endptr, 10);
            if (endptr == buf+i) {
                ERROR("Invalid size field in object");
                return -1;
            } else {
                break;
            }
        }
    }

    if (obj->type == OBJECT_TYPE_UNKNOWN || obj->size == -1) {
        ERROR("Unable to parse object header");
        return -1;
    }

    return 0;
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

    strcat(path, objects_dir);
    strcat(path, hash+2);

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

    obj->fd = open(path, (create ? O_RDWR : O_RDONLY) | O_CREAT, default_mode);
    if (obj->fd < 0) {
        ERROR("Unable to open %s: %s", path, strerror(errno));
        return -1;
    }

    /* Size and type unknown */
    obj->size = -1;
    obj->type = OBJECT_TYPE_UNKNOWN;

    setup_zlib_inflate(obj);

    return 0;
}

int object_close(struct object *obj) {
    close(obj->fd);
    if (inflateEnd(&obj->strm) != Z_OK) {
        return -1;
    }

    return 0;
}

ssize_t object_read(struct object *obj, uint8_t *buf, size_t count) {
    ssize_t status;
    int ret = 0;

    obj->strm.avail_out = count;
    obj->strm.next_out = buf;

    if (obj->strm.avail_in == 0) {
        status = read(obj->fd, obj->comp_buf, OBJECT_READ_BUF_SZ);

        if (status < 0) {
            ERROR("Failed to read compressed object data: %s",
                  strerror(errno));
            return -1;
        }

        /* EOF */
        if (status == 0)
            return 0;

        obj->strm.avail_in = (size_t)status;
        obj->strm.next_in = obj->comp_buf;

    }

    while (ret != Z_STREAM_END && obj->strm.avail_out > 0) {
        ret = inflate(&obj->strm, Z_NO_FLUSH);
        assert(ret != Z_STREAM_ERROR);
        switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;     /* and fall through */
                __attribute__ ((fallthrough));
            case Z_DATA_ERROR:
                __attribute__ ((fallthrough));
            case Z_MEM_ERROR:
                ERROR("Decompressing object failed");
                return -1;
        }
    }

    if (obj->type == OBJECT_TYPE_UNKNOWN && obj->size == -1) {
        int header_sz = 0;
        if (parse_object_header(obj, buf, count)) {
            return -1;
        }

        /* Since we're sneakily using the caller's buffer to inflate all data,
         * we need to strip the header from the buffer on the first read */
        for (header_sz = 0; buf[header_sz] != 0; header_sz++);
        memmove(buf, buf+header_sz+1, count - obj->strm.avail_out - header_sz);
        count -= header_sz;
    }

    return count - obj->strm.avail_out;
}

static const char *object_type_string(enum object_type type) {
    return object_type_strings[type];
}

char *object_write(struct object *obj,
                   enum object_type type,
                   const char *file,
                   int write_to_db)
{
    SHA1_CTX context;
    char header[MIN_OBJECT_HEADER_SZ] = { 0 };
    uint8_t digest[20] = { 0 };
    uint8_t *buffer_in, *buffer_out;
    char digest_string[41];
    ssize_t header_sz = 0;
    int fd;
    int ret;
    int buffer_out_used;

    setup_zlib_deflate(obj, 6);

    ssize_t buffer_in_sz = file_size(file);
    /* This should be calculated better ..*/
    ssize_t buffer_out_sz = buffer_in_sz * 2;
    if (buffer_in_sz < 0) {
        return NULL;
    }

    fd = open(file, O_RDONLY);
    if (fd < 0) {
        ERROR("Can't open %s: %s", file, strerror(errno));
        return NULL;
    }

    header_sz = snprintf((char*)header, MIN_OBJECT_HEADER_SZ, "%s %ld",
                         object_type_string(type), buffer_in_sz);
    buffer_in_sz += header_sz;

    buffer_in = malloc(buffer_in_sz);
    buffer_out = malloc(buffer_out_sz);
    if (!buffer_in || !buffer_out) {
        ERROR("Failed to allocate memory for object compression");
    }

    strcpy((char*)buffer_in, header);

    assert(read(fd, buffer_in + header_sz + 1, buffer_in_sz - header_sz) ==
           buffer_in_sz - header_sz);

    obj->strm.avail_in = buffer_in_sz;
    obj->strm.next_in = buffer_in;
    obj->strm.avail_out = buffer_out_sz;
    obj->strm.next_out = buffer_out;

    ret = deflate(&obj->strm, Z_FINISH);
    assert(ret != Z_STREAM_ERROR);
    buffer_out_used = buffer_out_sz - obj->strm.avail_out;

    SHA1Init(&context);
    SHA1Update(&context, buffer_out, buffer_out_used);
    SHA1Final(digest, &context);
    SHA1DigestString(digest, digest_string);

    if (write_to_db) {
        int ret = object_open(obj, find_git_dir("."), digest_string, 1);

        if (!ret) {
            assert(write(obj->fd, buffer_out, buffer_out_used) ==
                   buffer_out_used);
        }
    }

    free(buffer_in);
    free(buffer_out);
    deflateEnd(&obj->strm);

    return strdup(digest_string);
}

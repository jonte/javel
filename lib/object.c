#include "limits.h"
#include "log.h"
#include "object.h"
#include "util.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MIN_OBJECT_HEADER_SZ 100

static int setup_zlib(struct object *obj) {
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

int object_open(struct object *obj, const char *git_dir, const char *hash) {
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "%s/objects/%c%c/%s", git_dir, hash[0], hash[1],
             hash+2);

    if (!is_file(path)) {
        ERROR("Object does not exist: %s", path);
    }

    obj->fd = open(path, O_RDONLY);
    if (obj->fd < 0) {
        ERROR("Unable to open %s: %s", path, strerror(errno));
        return -1;
    }

    /* Size and type unknown */
    obj->size = -1;
    obj->type = OBJECT_TYPE_UNKNOWN;

    return setup_zlib(obj);
}

int object_close(struct object *obj) {
    close(obj->fd);
    inflateEnd(&obj->strm);

    return 0;
}

ssize_t object_read(struct object *obj, uint8_t *buf, size_t count) {
    ssize_t status;
    int ret;

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
        memmove(buf, buf+header_sz, count - obj->strm.avail_out - header_sz);
        count -= header_sz;
    }

    return count - obj->strm.avail_out;
}

//ssize_t object_write(struct object *obj, const void *buf, size_t count) {
//    return 0;
//}

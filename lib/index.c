#include "index.h"
#include "logging.h"

#include <endian.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SUPPORTED_VERSION 2

static inline int read_field(int fd, void* dest, size_t size, int flip) {
    if(read(fd, dest, size) != size) {
        ERROR("Failed to read index stat entry");
        return -1;
    }

    if (!flip) {
        return 0;
    }

    switch (size) {
        case sizeof(uint16_t):
            *(uint16_t*)dest = be16toh(*(uint16_t*)dest);
            break;
        case sizeof(uint32_t):
            *(uint32_t*)dest = be32toh(*(uint32_t*)dest);
            break;
        case sizeof(uint64_t):
            *(uint64_t*)dest = be64toh(*(uint64_t*)dest);
            break;
        default: {
            uint8_t *new = malloc(size);
            for (int i = 0; i < size / sizeof(*new); i++) {
                new [size - i - 1] = ((uint8_t*)dest)[i];
            }

            memcpy(dest, new, size);
            free (new);
        }
    }

    return 0;
}

static int populate_files(struct index *idx) {
    if (read_field(idx->fd, &idx->files_len, sizeof(idx->files_len), 1)) {
        ERROR("Failed to read number of entries");
        return -1;
    }

    idx->files = calloc(sizeof(*idx->files), idx->files_len);
    if (!idx->files) {
        ERROR("Unable to allocate memory for index files");
        return -1;
    }

    for (int files_left = idx->files_len; files_left > 0; files_left--) {
        uint8_t padding = 0;
        int s = 0;
        struct index_file *ifile = &idx->files[idx->files_len - files_left];

        s |= read_field(idx->fd, &ifile->ctime_s, sizeof(ifile->ctime_s), 1);
        s |= read_field(idx->fd, &ifile->ctime_ns, sizeof(ifile->ctime_ns), 1);
        s |= read_field(idx->fd, &ifile->mtime_s, sizeof(ifile->mtime_s), 1);
        s |= read_field(idx->fd, &ifile->mtime_ns, sizeof(ifile->mtime_ns), 1);
        s |= read_field(idx->fd, &ifile->dev, sizeof(ifile->dev), 1);
        s |= read_field(idx->fd, &ifile->ino, sizeof(ifile->ino), 1);
        s |= read_field(idx->fd, &ifile->mode, sizeof(ifile->mode), 1);
        s |= read_field(idx->fd, &ifile->uid, sizeof(ifile->uid), 1);
        s |= read_field(idx->fd, &ifile->gid, sizeof(ifile->gid), 1);
        s |= read_field(idx->fd, &ifile->size, sizeof(ifile->size), 1);
        s |= read_field(idx->fd, &ifile->hash, sizeof(ifile->hash), 0);
        s |= read_field(idx->fd, &ifile->flags, sizeof(ifile->flags), 1);
        s |= read_field(idx->fd, &ifile->path, ifile->flags.name_length, 0);

        while (padding == 0 && read(idx->fd, &padding, 1) == 1);
        lseek(idx->fd, -1, SEEK_CUR);

        if (s) {
            ERROR("Failed to read index entry. Corrupt index?");
            return -1;
        }
    }

    return 0;
}

static int verify_header(struct index *idx) {
    uint8_t magic[4 /* DIRC is the expected magic */];
    uint32_t version;

    if (read(idx->fd, magic, sizeof(magic)) != sizeof(magic)) {
        ERROR("Failed to read index");
        return -1;
    }

    if (memcmp(magic, "DIRC", sizeof(magic))) {
        ERROR("Index corrupt, header magic not found");
        return -1;
    }

    if (read(idx->fd, &version, sizeof(version)) != sizeof(version)) {
        ERROR("Index corrupt, version not found");
        return -1;
    }

    if ((version = be32toh(version)) != SUPPORTED_VERSION) {
        ERROR("Unsupported index version");
        return -1;
    }

    return 0;
}

int index_open(struct index *idx, const char *git_dir) {
    char path[PATH_MAX];

    snprintf(path, sizeof(path), "%s/index", git_dir);

    idx->fd = open(path, O_RDWR);
    if (idx->fd < 0) {
        ERROR("Failed to open index");
        return -1;
    }

    if (verify_header(idx)) {
        return -1;
    }

    if (populate_files(idx)) {
        return -1;
    }

    return 0;
}

int index_close(struct index *idx) {
    free(idx->files);
    return 0;
}

int index_next_file(struct index *idx, const struct index_file **ifile) {
    if (idx->files_iter < idx->files_len) {
        *ifile = &idx->files[idx->files_iter++];
    } else {
        return -1;
    }

    return 0;
}

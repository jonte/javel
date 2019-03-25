#pragma once

#include <limits.h>
#include <stdint.h>
#include <sys/stat.h>

struct index {
    int fd;
    uint32_t files_len;
    struct index_file *files;
    uint32_t files_iter;
};

#pragma pack (push, 1)
struct index_file {
    uint32_t ctime_s;
    uint32_t ctime_ns;
    uint32_t mtime_s;
    uint32_t mtime_ns;
    uint32_t dev;
    uint32_t ino;
    struct {
        unsigned int permission: 9;
        unsigned int : 3; /* Unused */
        unsigned int object_type: 4;
        unsigned int : 16; /* Padding */
    } mode;
    uint32_t uid;
    uint32_t gid;
    uint32_t size;
    uint8_t hash[20];
    struct {
        unsigned int name_length: 12;
        unsigned int stage: 2;
        unsigned int extended: 1;
        unsigned int assume_valid: 1;
    } flags;
    char path[PATH_MAX];
};
#pragma pack (pop)

enum {
    INDEX_FILE_REGULAR = 8,
    INDEX_FILE_SYMLINK = 10,
    INDEX_FILE_GITLINK = 14,
};

int index_open(struct index *idx, const char *git_dir);
int index_close(struct index *idx);
int index_next_file(struct index *idx,
                    const struct index_file **idx_file);

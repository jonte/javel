#include "config.h"
#include "log.h"
#include "util.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

static const mode_t default_mode =
    S_IRUSR | S_IWUSR | S_IXUSR |
    S_IRGRP |           S_IXGRP |
                        S_IXOTH;

static int write_file(char *fname, char *content, ssize_t size) {
    int fd;

    fd = open(fname, O_RDWR | O_CREAT, default_mode);
    if (fd > 0) {
        if (write(fd, content, size) != size) {
            ERROR("Failed to write %s: %s", fname, strerror(errno));
            return -1;
        }
    } else {
        ERROR("Failed to open %s: %s", fname, strerror(errno));
        return -1;
    }

    return 0;
}

static int create_git_dirs() {
    int ret = 0;

    ret |= mkdir(".git", default_mode);
    ret |= mkdir(".git/branches", default_mode);
    ret |= mkdir(".git/objects", default_mode);
    ret |= mkdir(".git/objects/info", default_mode);
    ret |= mkdir(".git/objects/pack", default_mode);
    ret |= mkdir(".git/refs", default_mode);
    ret |= mkdir(".git/refs/tags", default_mode);
    ret |= mkdir(".git/refs/heads", default_mode);

    if (ret) {
        ERROR("Failed to create .git directory or contents: %s",
              strerror(errno));
        return -1;
    }

    return 0;
}

static int write_default_config() {
    struct config config = { 0 };
    int fd;

    config_set(&config, "core", "repositoryformatversion", "0");
    config_set(&config, "core", "filemode", "false");
    config_set(&config, "core", "bare", "false");

    fd = open(".git/config", O_RDWR | O_CREAT, default_mode);
    if (!fd) {
        ERROR("Failed to open .git/config: %s", strerror(errno));
        return -1;
    }

    if (config_write_to_file(&config, fd)) {
        config_destroy(&config);
        return -1;
    }

    config_destroy(&config);

    return 0;
}

int jvl_init(char *dir) {
    if (!is_dir(dir)) {
        if (mkdir(dir, default_mode)) {
            ERROR("Failed to create directory %s: %s", dir, strerror(errno));
        }
        return -1;
    }

    if (num_entries_in_dir(dir) > 0) {
        ERROR("%s is not empty", dir);
        return -1;
    }

    if (chdir(dir)) {
        ERROR("Failed to chdir to %s: %s", dir, strerror(errno));
    }

    if (create_git_dirs()) {
        return -1;
    }

    char desc[] = "Unnamed repository; edit this file 'description' to "
        "name the repository\n";
    if (write_file(".git/description", desc, sizeof(desc))) {
        return -1;
    }

    char head[] = "ref: refs/heads/master";
    if (write_file(".git/HEAD",head, sizeof(head))) {
        return -1;
    }

    if (write_default_config()) {
        return -1;
    }

    return 0;
}

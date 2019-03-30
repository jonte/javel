#include "logging.h"

#include <dirent.h>
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int is_dir(const char *dir) {
    struct stat sb;

    return stat(dir, &sb) == 0 && S_ISDIR(sb.st_mode);
}

int is_file(const char *file) {
    struct stat sb;

    return stat(file, &sb) == 0 && S_ISREG(sb.st_mode);
}

int file_size(const char *file) {
    struct stat sb;

    if (stat(file, &sb)) {
        ERROR("Unable to stat %s: %s", file, strerror(errno));
        return -1;
    }

    return sb.st_size;
}

char *resolve_ref(const char *git_dir, const char *ref) {
    char path[PATH_MAX];
    char buf[1024] = { 0 };
    int fd;
    int ref_sz;

    snprintf(path, PATH_MAX, "%s/%s", git_dir, ref);
    fd = open(path, O_RDONLY);
    if (fd < 0) {
        ERROR("Failed to open ref %s: %s", path, strerror(errno));
        return NULL;
    }

    if ((ref_sz = read(fd, buf, sizeof(buf))) == sizeof(buf)) {
        ERROR("Ref too long");
        return NULL;
    }

    if (!strncmp(buf, "ref: ", 5)) {
        if (buf[ref_sz-1] == '\n') {
            buf[ref_sz-1] = '\0';
        }

        return resolve_ref(git_dir, buf + 5);
    } else {
        if (buf[40] == '\n') {
            buf[40] = '\0';
        }
        return strndup(buf, sizeof(buf));
    }
}

int update_ref(const char *git_dir, const char *ref_name, const char *hash) {
    char path[PATH_MAX];
    int fd;

    snprintf(path, PATH_MAX, "%s/refs/%s", git_dir, ref_name);
    fd = open(path, O_WRONLY | O_TRUNC | O_CREAT, 0644);
    if (fd < 0) {
        ERROR("Failed to open ref %s: %s", path, strerror(errno));
        return -1;
    }

    if (write(fd, hash, 41) != 41) {
        ERROR("Failed to write ref: %s", strerror(errno));
        return -1;
    }

    return 0;
}

int num_entries_in_dir(const char *dir) {
    int entry_count = -2; // Don't count . and ..
    DIR * dirp;
    struct dirent * entry;

    dirp = opendir(dir);

    if (!dirp) {
        perror("opendir");
        return -1;
    }

    while ((entry = readdir(dirp)) != NULL) {
        entry_count++;
        DBG("Directory entry: %s", entry->d_name);
    }

    closedir(dirp);

    return entry_count;
}

char *find_git_dir(const char *dir) {
    char check_path[PATH_MAX];
    char parent_path[PATH_MAX];
    snprintf(check_path, PATH_MAX, "%s/.git", dir);

    if (is_dir(check_path)) {
        return strndup(check_path, PATH_MAX);
    }

    snprintf(check_path, PATH_MAX, "%s/..", dir);
    if (!realpath(check_path, parent_path)) {
        ERROR ("Failed to recurse to parent directory");
        return NULL;
    }

    if (!strncmp(dir, parent_path, PATH_MAX)) {
        /* There is no .git directory before reaching / */
        return NULL;
    }

    return find_git_dir(parent_path);
}

char *get_head(const char *dir) {
    char *git_dir = find_git_dir(dir);
    char *ref = resolve_ref(git_dir, "HEAD");
    free(git_dir);

    return ref;
}

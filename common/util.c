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

int resolve_ref(const char *git_dir, const char *ref, char *resolved_ref_out) {
    char path[PATH_MAX];
    char buf[1024] = { 0 };
    int fd;
    int ref_sz;

    snprintf(path, PATH_MAX, "%s/%s", git_dir, ref);
    fd = open(path, O_RDONLY);
    if (fd < 0) {
        ERROR("Failed to open ref %s: %s", path, strerror(errno));
        return -1;
    }

    if ((ref_sz = read(fd, buf, sizeof(buf))) == sizeof(buf)) {
        ERROR("Ref too long");
        return -1;
    }

    if (!strncmp(buf, "ref: ", 5)) {
        if (buf[ref_sz-1] == '\n') {
            buf[ref_sz-1] = '\0';
        }

        return resolve_ref(git_dir, buf + 5, resolved_ref_out);
    } else {
        if (buf[40] == '\n') {
            buf[40] = '\0';
        }
        strncpy(resolved_ref_out, buf, PATH_MAX);
        return 0;
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

int find_git_dir(const char *dir, char *git_dir_out) {
    char check_path[PATH_MAX];
    char parent_path[PATH_MAX];
    snprintf(check_path, PATH_MAX, "%s/.git", dir);

    if (is_dir(check_path)) {
        strncpy(git_dir_out, check_path, PATH_MAX);
        return 0;
    }

    snprintf(check_path, PATH_MAX, "%s/..", dir);
    if (!realpath(check_path, parent_path)) {
        ERROR ("Failed to recurse to parent directory");
        return -1;
    }

    if (!strncmp(dir, parent_path, PATH_MAX)) {
        /* There is no .git directory before reaching / */
        return -1;
    }

    return find_git_dir(parent_path, git_dir_out);
}

int find_root(const char *dir, char *root_dir_out) {
    if (find_git_dir(dir, root_dir_out)) {
        return -1;
    }

    int root_dir_len = strlen(root_dir_out);
    root_dir_out[root_dir_len-4] = '\0';

    return 0;
}

int find_in_root(const char *dir, const char *file, char *path_out) {
    if (find_root(dir, path_out)) {
        return -1;
    }

    strncat(path_out, file, PATH_MAX);
    return 0;
}

int get_head(const char *dir, char *ref_out) {
    char git_dir[PATH_MAX];
    if (find_git_dir(dir, git_dir)) {
        return -1;
    }

    return resolve_ref(git_dir, "HEAD", ref_out);
}

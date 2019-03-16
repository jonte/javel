#include "log.h"

#include <dirent.h>
#include <errno.h>
#include <error.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

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

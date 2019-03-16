#include "log.h"

#include <dirent.h>
#include <error.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/stat.h>

int is_dir(char *dir) {
    struct stat sb;

    return stat(dir, &sb) == 0 && S_ISDIR(sb.st_mode);
}

int num_entries_in_dir(char *dir) {
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

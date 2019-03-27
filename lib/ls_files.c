#include "index.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>

int jvl_ls_files() {
    const struct index_file *idx_file = NULL;
    struct index idx = { 0 };

    char *git_dir = find_git_dir(".");

    if (index_open(&idx, git_dir)) {
        return -1;
    }

    while (!index_next_file(&idx, &idx_file)) {
        printf("%s\n", idx_file->path);
    }

    index_close(&idx);
    free(git_dir);

    return 0;
}

#define _DEFAULT_SOURCE
#include "commit.h"
#include "commit-object.h"
#include "common.h"
#include "config.h"
#include "logging.h"
#include "show.h"
#include "util.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

static char *default_ignore[] = {
    ".",
    "..",
    ".git"
};

static char **custom_ignore = NULL;
static size_t custom_ignore_len;

static int read_custom_ignore(const char *git_dir) {
    struct stat sb = { 0 };
    char gitignore[PATH_MAX];
    char *gitignore_buf;
    ssize_t gitignore_buf_sz;
    int fd;

    if (find_in_root(git_dir, ".gitignore", gitignore)) {
        return -1;
    }

    lstat(gitignore, &sb);

    fd = open(gitignore, O_RDONLY);
    if (fd < 0) {
        DBG("Failed to open gitignore: %s", strerror(errno));
        return -1;
    }

    gitignore_buf_sz = sb.st_size;

    gitignore_buf = malloc(gitignore_buf_sz);
    if (!gitignore_buf) {
        ERROR("Failed to alloc buffer for gitignore: %s", strerror(errno));
        return -1;
    }

    if (read(fd, gitignore_buf, gitignore_buf_sz) != gitignore_buf_sz) {
        ERROR("Failed to read gitignore: %s", strerror(errno));
        return -1;
    }

    int i;
    char *str;
    for(i = 1, str = gitignore_buf;;i++, str = NULL) {
        char *line = strtok(str, "\n");
        if (!line) {
            break;
        }

        custom_ignore = realloc(custom_ignore, i * sizeof(*custom_ignore));
        if (!custom_ignore) {
            ERROR("Failed to allocate data structure for gitignore: %s",
                  strerror(errno));
            free (gitignore_buf);
            return -1;
        }

        custom_ignore[i - 1] = strndup(line, PATH_MAX);
    }

    custom_ignore_len = i - 1;

    free(gitignore_buf);

    return 0;
}

static int match_expr(const char *expr, const char *fname) {
    /* TODO: Impement matching algorithm */
    return !strcmp(expr, fname);
}

static int ignore_filter(const struct dirent *dirent) {
    size_t num_entries = sizeof(default_ignore) / sizeof(*default_ignore);

    for (size_t i = 0; i < num_entries; i++) {
        if (!strcmp(dirent->d_name, default_ignore[i]))
        {
            return 0;
        }
    }

    for (size_t i = 0; i < custom_ignore_len; i++) {
        if (match_expr(custom_ignore[i], dirent->d_name)) {
            return 0;
        }
    }

    return 1;
}

static int walk(const char *path, struct tree_object *tree, const char *git_dir)
{
    char *hash = NULL;
    struct dirent **entries;
    int nents;

    nents = scandir(path, &entries, ignore_filter, alphasort);
    if (nents < 0) {
        ERROR("scandir failed: %s\n", strerror(errno));
        return 1;
    }

    for (int i = 0; i < nents; i++) {
        struct dirent *entry = entries[i];
        char newpath[PATH_MAX] = { 0 };

        snprintf(newpath, PATH_MAX, "%s/%s", path, entry->d_name);

        switch (entry->d_type) {
            case DT_DIR: {
                struct tree_object inner_tree = { 0 };
                walk(newpath, &inner_tree, git_dir);
                hash = tree_object_write(&inner_tree, git_dir);
                tree_object_add_entry(tree,
                                      040000, /*TODO: Put real mode */
                                      hash,
                                      entry->d_name,
                                      OBJECT_TYPE_TREE);
                break;
            } case DT_REG: {
                struct object obj = { 0 };
                hash = object_write(&obj, OBJECT_TYPE_BLOB, newpath, 1);
                object_close(&obj);
                tree_object_add_entry(tree,
                                      0644, /*TODO: Put real mode */
                                      hash,
                                      entry->d_name,
                                      OBJECT_TYPE_BLOB);
                free (hash);
                break;
            } default:
                ERROR("Unsupported entry: %s", newpath);

        }
        free(entries[i]);
    }

    free(entries);

    return 0;
}

static int read_config(struct config *config, struct identity *author,
                       struct identity *committer, const char *git_dir)
{
    char config_file_path[PATH_MAX];
    const char *name = NULL;
    const char *email = NULL;

    snprintf(config_file_path, sizeof(config_file_path), "%s/config", git_dir);

    if (!config_from_file(config, config_file_path)) {
        name = config_read(config, "user.name");
        email = config_read(config, "user.email");
    }

    if (!name) {
        author->name = strdup("Unknown author");
        committer->name = strdup("Unknown committer");
    } else {
        author->name = strdup(name);
        committer->name = strdup(name);
    }

    if (!name) {
        author->email = strdup("Unknown email");
        committer->email = strdup("Unknown email");
    } else {
        committer->email = strdup(email);
        author->email = strdup(email);
    }


    return 0;
}

int jvl_commit(int argc, char **argv) {
    if (argc != 2) {
        ERROR("Command '%s' failed: The only allowed parameter is MESSAGE",
              argv[0]);
        return -1;
    }

    const char *message = argv[1];
    char root[PATH_MAX] = { 0 };
    struct commit_object commit_obj = { 0 };
    struct tree_object tree_obj = { 0 };
    char git_dir[PATH_MAX];
    char *hash = NULL;
    struct identity author = { 0 }, committer = { 0 };
    struct config config = { 0 };
    char ref[REF_MAX];
    time_t current_time;
    struct tm *time_info;

    if (find_git_dir(".", git_dir)) {
        return -1;
    }

    if (read_config(&config, &author, &committer, git_dir)) {
        return -1;
    }

    read_custom_ignore(git_dir);

    time(&current_time);
    time_info = localtime(&current_time);

    author.date = time(NULL);
    committer.date = time(NULL);
    author.tz = (time_info->tm_gmtoff / 60 / 60) * 100 +
        (time_info->tm_gmtoff / 60) % (time_info->tm_gmtoff > 0 ? 60 : -60);
    committer.tz = (time_info->tm_gmtoff / 60 / 60) * 100 +
        (time_info->tm_gmtoff / 60) % (time_info->tm_gmtoff > 0 ? 60 : -60);

    config_destroy(&config);

    if (resolve_ref(git_dir, "HEAD", ref)) {
        strcpy(ref, "");
    }

    find_root(git_dir, root);

    walk(root, &tree_obj, git_dir);
    hash = tree_object_write(&tree_obj, git_dir);

    commit_object_new(&commit_obj,
                      ref,
                      hash,
                      &author,
                      &committer,
                      message);
    free(hash);

    hash = commit_object_write(&commit_obj, git_dir);

    /* TODO: Support branches */
    update_ref(git_dir, "heads/master", hash);

    free(author.email);
    free(author.name);
    free(committer.email);
    free(committer.name);

    return 0;
}

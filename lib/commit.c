#define _DEFAULT_SOURCE
#include "commit.h"
#include "commit-object.h"
#include "config.h"
#include "logging.h"
#include "show.h"
#include "util.h"

#include <dirent.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

static int walk(const char *path, struct tree_object *tree, const char *git_dir)
{
    char *hash = NULL;
    DIR * dirp;
    struct dirent * entry;

    dirp = opendir(path);

    if (!dirp) {
        perror("opendir");
        return -1;
    }

    while ((entry = readdir(dirp)) != NULL) {
        char newpath[PATH_MAX] = { 0 };
        if (!strncmp(entry->d_name, ".", 2)  ||
            !strncmp(entry->d_name, "..", 3) ||
            !strncmp(entry->d_name, ".git", 5))
        {
            continue;
        }

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
    }

    closedir(dirp);
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

int jvl_commit(const char *message) {
    char root[PATH_MAX] = { 0 };
    struct commit_object commit_obj = { 0 };
    struct tree_object tree_obj = { 0 };
    char *git_dir = find_git_dir(".");
    char *hash = NULL;
    struct identity author = { 0 }, committer = { 0 };
    struct config config = { 0 };
    char *ref = NULL;
    time_t current_time;
    struct tm *time_info;

    if (read_config(&config, &author, &committer, git_dir)) {
        return -1;
    }

    time(&current_time);
    time_info = localtime(&current_time);

    author.date = time(NULL);
    committer.date = time(NULL);
    author.tz = (time_info->tm_gmtoff / 60 / 60) * 100 +
        (time_info->tm_gmtoff / 60) % (time_info->tm_gmtoff > 0 ? 60 : -60);
    committer.tz = (time_info->tm_gmtoff / 60 / 60) * 100 +
        (time_info->tm_gmtoff / 60) % (time_info->tm_gmtoff > 0 ? 60 : -60);

    config_destroy(&config);

    if (!git_dir) {
        ERROR("Not a git repository");
        return -1;
    }

    ref = resolve_ref(git_dir, "HEAD");
    if (!ref) {
        ref = "";
    }

    snprintf(root, PATH_MAX, "%s/../", git_dir);

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

    free(git_dir);
    free(author.email);
    free(author.name);
    free(committer.email);
    free(committer.name);

    return 0;
}

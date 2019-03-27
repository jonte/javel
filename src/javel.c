#include "cat-file.h"
#include "checkout.h"
#include "commit.h"
#include "hash-file.h"
#include "init.h"
#include "logging.h"
#include "log.h"
#include "ls_files.h"
#include "ls-tree.h"
#include "show.h"
#include "status.h"
#include "util.h"

#include <argp.h>
#include <stdlib.h>
#include <string.h>

static char doc[] =
"Jävel - A Git implementation\n\n"
"Commands:\n"
"init\n"
"  - Initialize a new git directory\n"
"cat-file HASH\n"
"  - Cat the contents of HASH to stdout\n"
"hash-file FILE\n"
"  - Hash a file, and store the resulting object\n"
"show [HASH]\n"
"  - Show the contents of a commit\n"
"log [HASH]\n"
"  - Show the log ending with HASH (or HEAD per default)\n"
"ls-tree HASH\n"
"  - Show a list of objects referenced by the tree object HASH\n"
"checkout HASH DIR\n"
"  - Check out the commit HASH in the directory DIR\n"
"commit MESSAGE\n"
"  - Commit all changes, with the mesage MESSAGE\n"
"\n"
"Options:";

static char args_doc[] = "COMMAND [COMMAND...]";

struct arguments
{
    char *args[3];
};

static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
    struct arguments *arguments = state->input;

    switch (key) {
        case ARGP_KEY_NO_ARGS:
            argp_usage (state);
            break;

        case ARGP_KEY_ARG:
            if (state->arg_num >= 3)
                argp_usage (state);

            arguments->args[state->arg_num] = arg;
            break;

        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

static struct argp argp = { 0, parse_opt, args_doc, doc, 0, 0, 0};

#define IS_ARG(arg1, arg2)\
    (!strncmp((arg1), (arg2), strlen(arg2)))

static char *get_head(const char *dir) {
    char *git_dir = find_git_dir(dir);
    char *ref = resolve_ref(git_dir, "HEAD");
    free(git_dir);

    return ref;
}

int main(int argc, char **argv) {
    struct arguments arguments = { 0 };

    argp_parse (&argp, argc, argv, 0, 0, &arguments);

    if (IS_ARG(arguments.args[0], "init")) {
        if (!arguments.args[1]) {
            arguments.args[1] = ".";
        }

        return jvl_init(arguments.args[1]);
    } else if (IS_ARG(arguments.args[0], "cat-file")) {
        if (!arguments.args[1]) {
            ERROR("Second argument must be a valid commit hash");
            return -1;
        }

        return jvl_cat_file(arguments.args[1]);
    } else if (IS_ARG(arguments.args[0], "hash-file")) {
        if (!arguments.args[1]) {
            ERROR("Second argument must be a valid file name");
            return -1;
        }

        return jvl_hash_file(arguments.args[1]);
    } else if (IS_ARG(arguments.args[0], "show")) {
        int ret;
        if (arguments.args[1]) {
            ret = jvl_show(arguments.args[1]);
        } else {
            char *ref = get_head(".");
            ret = jvl_show(ref);
            free(ref);
        }

        return ret;
    } else if (IS_ARG(arguments.args[0], "log")) {
        int ret;
        if (!arguments.args[1]) {
            char *ref = get_head(".");
            ret = jvl_log(ref);
            free(ref);
        } else {
            ret = jvl_log(arguments.args[1]);
        }

        return ret;
    } else if (IS_ARG(arguments.args[0], "ls-tree")) {
        if (!arguments.args[1]) {
            ERROR("Second argument must be a valid commit hash");
            return -1;
        }

        return jvl_ls_tree(arguments.args[1]);
    } else if (IS_ARG(arguments.args[0], "checkout")) {
        if (!arguments.args[1]) {
            ERROR("Second argument must be a valid commit hash");
            return -1;
        }

        if (!arguments.args[2]) {
            ERROR("Third argument must be an empty directory to checkout in");
            return -1;
        }

        return jvl_checkout(arguments.args[1], arguments.args[2]);
    } else if (IS_ARG(arguments.args[0], "commit")) {
        if (!arguments.args[1]) {
            ERROR("Second argument must be a commit message (within quotes)");
            return -1;
        }

        return jvl_commit(arguments.args[1]);
    } else if (IS_ARG(arguments.args[0], "ls-files")) {
        return jvl_ls_files();
    } else if (IS_ARG(arguments.args[0], "status")) {
        return jvl_status();
    } else {
        fprintf(stderr, "Unknown argument '%s'\n", arguments.args[0]);
    }

    return 0;
}

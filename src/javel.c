#include "show.h"
#include "hash-file.h"
#include "cat-file.h"
#include "init.h"
#include "log.h"

#include <argp.h>
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
"show HASH\n"
"  - Show the contents of a commit\n"
"\n"
"Options:";

static char args_doc[] = "COMMAND [COMMAND...]";

struct arguments
{
    char *args[2];
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
            if (state->arg_num >= 2)
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
        if (!arguments.args[1]) {
            ERROR("Second argument must be a valid commit hash");
            return -1;
        }

        return jvl_show(arguments.args[1]);
    } else {
        fprintf(stderr, "Unknown argument '%s'\n", arguments.args[0]);
    }

    return 0;
}

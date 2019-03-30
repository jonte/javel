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

typedef int (*command_fun_t)(int, char **);

struct arguments {
    char **sub_args;
    int sub_args_len;
    command_fun_t f;
};

struct command_entry {
    char *fname;
    command_fun_t f;
    char *help_args;
    char *help_message;
};

static struct command_entry commands[] = {
    {"show",      jvl_show,      "COMMIT",   "Show the content of COMMIT"},
    {"status",    jvl_status,    "",         "Show the current index status"},
    {"init",      jvl_init,      "[DIR]",    "Initialize a new git directory in DIR"},
    {"cat-file",  jvl_cat_file,  "FILE",     "Cat the contents of HASH to stdout"},
    {"hash-file", jvl_hash_file, "FILE",     "Hash FILE, and store the resulting object"},
    {"log",       jvl_log,       "[HASH]",   "Show the log ending with HASH (or HEAD per default)"},
    {"ls-tree",   jvl_ls_tree,   "HASH",     "Show objects referenced by the tree object HASH"},
    {"ls-files",  jvl_ls_files,  "",         "Show files currently tracked by jvl"},
    {"checkout",  jvl_checkout,  "HASH DIR", "Check out the commit HASH in the directory DIR"},
    {"commit",    jvl_commit,    "MESSAGE",  "Commit all changes, with the mesage MESSAGE"},
};



static char banner[] = "Jävel - A Git implementation";
static char doc[sizeof(commands) +
                sizeof(banner) +
                1024 /* Room for whitespace */] = { 0 };
static char args_doc[] = "COMMAND";

static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
    struct arguments *arguments = state->input;

    switch (key) {
        case ARGP_KEY_NO_ARGS:
            argp_usage (state);
            break;

        case ARGP_KEY_ARG:
            arguments->sub_args = &state->argv[state->next - 1];
            arguments->sub_args_len = state->argc - state->next + 1;
            state->next = state->argc;

            for (size_t i = 0; i < sizeof(commands) / sizeof(*commands); i++) {
                const struct command_entry *entry = &commands[i];
                if (!strcmp(entry->fname, arg)) {
                    arguments->f = entry->f;
                    break;
                }
            }

            if (!arguments->f) {
                argp_error(state, "%s is not a known command\n", arg);
                return -1;
            }
            break;

        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

static struct argp argp = { 0, parse_opt, args_doc, doc, 0, 0, 0};

void build_doc_string() {
    size_t off = 0;
    off += snprintf(doc, sizeof(doc) - off - 1, "%s\n\n", banner);

    for (size_t i = 0; i < sizeof(commands) / sizeof(*commands); i++) {
        const struct command_entry *cmd = &commands[i];
        off += snprintf(doc + off, sizeof(doc) - off - 1,
                        "%8s%-10s%-8s%s\n",
                        "", cmd->fname, cmd->help_args, cmd->help_message);
    }
}

int main(int argc, char **argv) {
    struct arguments arguments = { 0 };

    build_doc_string();

    argp_parse (&argp, argc, argv, 0, 0, &arguments);

    return arguments.f(arguments.sub_args_len, arguments.sub_args);
}

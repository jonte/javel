#include <stddef.h>

struct config_entry {
    char *section;
    char *key;
    char *value;
};

struct config {
    struct config_entry *entries;
    size_t num_entries;
};

int config_set(struct config *config,
               const char *section,
               const char *key,
               const char *value);

int config_write_to_file(struct config *config, int fd);

void config_destroy(struct config *config);

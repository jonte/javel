#include "config.h"
#include "log.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static struct config_entry *config_alloc_entry(struct config *config) {
    config->entries = realloc(config->entries, ++config->num_entries *
                              sizeof(*config->entries));
    if (!config->entries) {
        ERROR("Failed to allocate new config entry");
        return NULL;
    }

    return &config->entries[config->num_entries - 1];
}

int config_set(struct config *config,
               const char *section,
               const char *key,
               const char *value) {
    struct config_entry *entry = config_alloc_entry(config);
    if (!entry) {
        ERROR("Failed to add new section to config");
        return -1;
    }

    if (section) {
        entry->section = strdup(section);
    }

    if (key) {
        entry->key = strdup(key);
    }

    if (value) {
        entry->value = strdup(value);
    }

    if (!entry->section || (!entry->key && key) || (!entry->value && value)) {
        ERROR("Failed to populate new section in config");
        free (entry->section);
        free (entry->key);
        free (entry->value);
        free (entry);
        return -1;
    }

    return 0;
}

int config_write_to_file(struct config *config, int fd) {
    char *prev_section = NULL;

    for (size_t i = 0; i < config->num_entries; i++) {
        char *section = config->entries[i].section;
        char *key = config->entries[i].key;
        char *value = config->entries[i].value;

        if (!prev_section || strcmp(prev_section, section)) {
            if (dprintf(fd, "[%s]\n", section) < (int)strlen(section) + 3) {
                ERROR("Failed to write config file section");
                return -1;
            }
        }

        if (dprintf(fd, "\t %s = %s\n", key, value) <
            (int)(strlen(key) + strlen(value) + 6))
        {
            ERROR("Failed to write config file key-value pair");
            return -1;
        }
        prev_section = section;
    }

    return 0;
}

void config_destroy(struct config *config) {
    for (size_t i = 0; i < config->num_entries; i++) {
        free(config->entries[i].section);
        free(config->entries[i].key);
        free(config->entries[i].value);
    }

    free(config->entries);
}

#include "config.h"
#include "logging.h"
#include "util.h"

#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CONFIG_LINE_MAX 4096

static struct config_entry *config_alloc_entry(struct config *config) {
    config->entries = realloc(config->entries, ++config->num_entries *
                              sizeof(*config->entries));
    if (!config->entries) {
        ERROR("Failed to allocate new config entry");
        return NULL;
    }

    return &config->entries[config->num_entries - 1];
}

int config_from_file(struct config *config, const char *file) {
    int fd = open(file, O_RDONLY);
    ssize_t r;
    char line[CONFIG_LINE_MAX];
    char *line_ptr = line;
    char c;
    char *section = NULL;

    if (fd < 0) {
        ERROR("Failed to open config %s: %s", file, strerror(errno));
        return -1;
    }

    while ((r = read(fd, &c, 1)) == 1) {
        if (c == '\n') {
            if (line[0] == '[') {
                char *tmp_sec = NULL;
                free (section);
                tmp_sec = strtok(line, "[]");
                if (tmp_sec) {
                    section = strdup(tmp_sec);
                }
            }

            char *value = strstr(line, "\"");
            char *key = strtok(line, "= \t\n");
            if (value) {
                value = strtok(value, "\"");
            } else{
                value = strtok(NULL, "= \t\n");
            }

            if (!key || !value) {
                line_ptr = line;
                memset(line, '\0', sizeof(line));
                continue;
            }

            struct config_entry *entry = config_alloc_entry(config);
            entry->section = strdup(section);
            entry->key = strdup(key);
            entry->value = strdup(value);

            line_ptr = line;
            memset(line, '\0', sizeof(line));
        }

        *line_ptr = c;
        line_ptr++;
    }

    close(fd);

    return 0;
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

const char *config_read(struct config *config, const char *identifier) {
    char *ident = strdup(identifier);
    char *section = strtok(ident, ".");
    char *key = strtok(NULL, ".");

    for (size_t i = 0; i < config->num_entries; i++) {
        struct config_entry *entry = &config->entries[i];
        if (!strcmp(entry->section, section) &&
            !strcmp(entry->key, key))
        {
            free (ident);
            return entry->value;
        }
    }

    free (ident);
    return NULL;
}

void config_destroy(struct config *config) {
    for (size_t i = 0; i < config->num_entries; i++) {
        free(config->entries[i].section);
        free(config->entries[i].key);
        free(config->entries[i].value);
    }

    free(config->entries);
}

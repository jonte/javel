#pragma once

int num_entries_in_dir (const char *dir);
int is_dir(const char *dir);
int is_file(const char *file);
char *find_git_dir(const char *dir);
int file_size(const char *file);
char *resolve_ref(const char *git_dir, const char *ref);
int update_ref(const char *git_dir, const char *ref_name, const char *hash);

#pragma once

int num_entries_in_dir (const char *dir);
int is_dir(const char *dir);
int is_file(const char *file);
int find_git_dir(const char *dir, char *git_dir_out);
int file_size(const char *file);
int resolve_ref(const char *git_dir, const char *ref, char *resolved_ref_out);
int update_ref(const char *git_dir, const char *ref_name, const char *hash);
int get_head(const char *dir, char *ref_out);
int find_root(const char *dir, char *root_dir_out);
int find_in_root(const char *dir, const char *file, char *path_out);

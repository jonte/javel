env = environment()
env.append('JVL', jvl.full_path())
find_program('git', required: true)

init_test = find_program('init-test.py')
test('init component test', init_test, env: env)

hash_file_test = find_program('hash-file-test.py')
test('hash-file component test', hash_file_test, env: env)

commit_test = find_program('commit-test.py')
test('commit component test', commit_test, env: env)

ls_files_test = find_program('ls-files-test.py')
test('ls-files component test', ls_files_test, env: env)

status_test = find_program('status-test.py')
test('status component test', status_test, env: env)

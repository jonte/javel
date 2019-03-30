env = environment()
env.append('JVL', jvl.full_path())
find_program('git', required: true)

suite = 'component tests'

init_test = find_program('init-test.py')
test('init', init_test, env: env, suite: suite)

hash_file_test = find_program('hash-file-test.py')
test('hash-file', hash_file_test, env: env, suite: suite)

commit_test = find_program('commit-test.py')
test('commit', commit_test, env: env, suite: suite)

ls_files_test = find_program('ls-files-test.py')
test('ls-files', ls_files_test, env: env, suite: suite)

status_test = find_program('status-test.py')
test('status', status_test, env: env, suite: suite)

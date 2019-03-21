env = environment()
env.append('JVL', jvl.full_path())
find_program('git', required: true)

init_test = find_program('init-test.py')
test('init component test', init_test, env: env)

hash_file_test = find_program('hash-file-test.py')
test('hash-file component test', hash_file_test, env: env)

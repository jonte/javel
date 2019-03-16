lib_include_dirs = include_directories('../lib/')

util_test_src = [
    'util_test.c',
    '../lib/util.c'
]

util_test = executable('util_test',
                       util_test_src,
                       include_directories: lib_include_dirs)

test('util test', util_test)

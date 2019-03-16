lib_include_dirs = include_directories('../lib/')

util_test = executable('util_test',
                       [
                           'util_test.c',
                           '../lib/util.c'
                       ],
                       include_directories: [inc, lib_include_dirs])

test('util test', util_test)

object_test = executable('object_test',
                         [
                             'object_test.c',
                             '../lib/object.c',
                             '../lib/sha1.c',
                             '../lib/util.c',
                         ],
                         include_directories: [inc, lib_include_dirs],
                         dependencies: zlib_dep)

test('object test', object_test)

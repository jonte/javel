lib_include_dirs = include_directories('../../lib/')

suite = 'unit tests'

util_test = executable('util_test',
                       [
                           'util_test.c',
                           '../../common/util.c'
                       ],
                       include_directories: [inc, lib_include_dirs])

test('util', util_test, suite: suite)

object_test = executable('object_test',
                         [
                             'object_test.c',
                             '../../lib/object.c',
                             '../../lib/sha1.c',
                             '../../common/util.c',
                         ],
                         include_directories: [inc, lib_include_dirs],
                         dependencies: zlib_dep)

test('object', object_test, suite: suite)

commit_object_test = executable('commit-object_test',
                                [
                                    'commit-object_test.c',
                                    '../../lib/commit-object.c',
                                    '../../lib/object.c',
                                    '../../lib/sha1.c',
                                    '../../common/util.c',
                                ],
                                include_directories: [inc, lib_include_dirs],
                                dependencies: zlib_dep)

test('commit-object', commit_object_test, suite: suite)

tree_object_test = executable('tree-object_test',
                              [
                                  'tree-object_test.c',
                                  '../../lib/commit-object.c',
                                  '../../lib/tree-object.c',
                                  '../../lib/object.c',
                                  '../../lib/sha1.c',
                                  '../../common/util.c',
                              ],
                              include_directories: [inc, lib_include_dirs],
                              dependencies: zlib_dep)

test('tree-object', tree_object_test, suite: suite)

index_test = executable('index_test',
                        [
                            '../../lib/index.c',
                            'index_test.c',
                            '../../common/util.c'
                        ],
                        include_directories: [inc, lib_include_dirs],
                        dependencies: zlib_dep)

test('index', index_test, suite: suite)

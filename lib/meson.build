lib_src = [
    'cat-file.c',
    'checkout.c',
    'commit.c',
    'commit-object.c',
    '../common/util.c',
    'config.c',
    'hash-file.c',
    'index.c',
    'init.c',
    'log.c',
    'ls_files.c',
    'ls-tree.c',
    'object.c',
    'sha1.c',
    'show.c',
    'status.c',
    'tree-object.c',
]

libjavel = shared_library('javel',
                          lib_src,
                          include_directories: inc,
                          dependencies: zlib_dep,
                          install: true)

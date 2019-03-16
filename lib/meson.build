lib_src = [
    'cat-file.c',
    'config.c',
    'hash-file.c',
    'init.c',
    'libjavel.c',
    'object.c',
    'sha1.c',
    'util.c',
]

libjavel = shared_library('javel',
                          lib_src,
                          include_directories: inc,
                          dependencies: zlib_dep,
                          install: true)

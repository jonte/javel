lib_src = [
    'config.c',
    'init.c',
    'libjavel.c',
    'object.c',
    'util.c',
    'cat-file.c',
]

libjavel = shared_library('javel',
                          lib_src,
                          include_directories: inc,
                          dependencies: zlib_dep,
                          install: true)

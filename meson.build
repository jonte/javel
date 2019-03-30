project('Jävel', 'c',
        version: '0.0.1',
        license: 'MIT',
        default_options: [
            'c_std=gnu11'
        ],
        meson_version: '>= 0.35.0'
)

if (get_option('english'))
    add_project_arguments('-DENGLISH', language : 'c')
endif

inc = include_directories(['include', 'common'])

zlib_dep = dependency('zlib', version : '>=1.2.8')

subdir('include')
subdir('lib')
subdir('src')
subdir('tests')
